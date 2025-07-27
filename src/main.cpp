// File Path: /src/main.cpp

#ifndef PIO_UNIT_TESTING

#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// --- Cabinet Headers ---
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <AdcManager.h>
#include <SdManager.h>
#include <INA219_Driver.h>
#include <PowerMonitor.h>
#include <FilterManager.h> // --- NEW: Include the new FilterManager cabinet ---

// =================================================================
// Global Objects
// =================================================================
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;
INA219_Driver ina219Driver;
PowerMonitor powerMonitor;
FilterManager filterManager; // --- NEW: Create a global instance of the FilterManager ---

SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;

// --- NEW: Global variables to hold the latest filtered sensor data ---
// These variables will be updated by the sensorTask and read by the telemetryTask.
double filtered_ph_voltage = 0.0;
double filtered_ec_voltage = 0.0;


// =================================================================
// FreeRTOS Task Prototypes
// =================================================================
void i2cTask(void* pvParameters);
void sensorTask(void* pvParameters);
void telemetryTask(void* pvParameters);

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  LOG_BOOT("SpHEC Meter v2.3.0 Booting...");

  // --- Initialize Core Systems & Buses ---
  spiMutex = xSemaphoreCreateMutex();
  if (spiMutex == NULL) {
      faultHandler.trigger_fault("SPI_MUTEX_FAIL", "Failed to create SPI mutex", __FILE__, __LINE__);
  }

  LOG_BOOT("Initializing shared VSPI bus...");
  vspi = new SPIClass(VSPI);
  vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
  LOG_BOOT("VSPI bus Initialized.");
  
  // --- Initialize Cabinets in Order ---
  LOG_BOOT("Initializing Core Cabinets...");
  faultHandler.begin();
  
  LOG_BOOT("Initializing DisplayManager...");
  if (!displayManager.begin(faultHandler)) {
      faultHandler.trigger_fault("DISPLAY_INIT_FAIL", "DisplayManager failed to init", __FILE__, __LINE__);
  }
  displayManager.showBootScreen();

  LOG_BOOT("Initializing AdcManager...");
  if (!adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN)) {
      faultHandler.trigger_fault("ADC_INIT_FAIL", "AdcManager failed to init", __FILE__, __LINE__);
  }
  LOG_BOOT("AdcManager Initialized.");

  LOG_BOOT("Priming SPI bus for SdManager...");
  adcManager.getVoltage(0);
  LOG_BOOT("SPI bus primed.");

  LOG_BOOT("Initializing SdManager...");
  if (!sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN)) {
      LOG_BOOT("WARNING: SdManager FAILED to initialize."); // Non-fatal
  }
  LOG_BOOT("SdManager Initialized.");

  LOG_BOOT("Initializing ConfigManager...");
  configManager.begin(faultHandler);
  LOG_BOOT("ConfigManager Initialized.");
  
  // --- NEW: Initialize the FilterManager ---
  LOG_BOOT("Initializing FilterManager...");
  if (!filterManager.begin(faultHandler, configManager)) {
      faultHandler.trigger_fault("FILTER_INIT_FAIL", "FilterManager failed to init", __FILE__, __LINE__);
  }
  LOG_BOOT("FilterManager Initialized.");

  LOG_BOOT("Initializing INA219_Driver...");
  if (!ina219Driver.begin(faultHandler)) {
      LOG_BOOT("WARNING: INA219_Driver FAILED to initialize."); // Non-fatal
  }
  LOG_BOOT("INA219_Driver Initialized.");

  LOG_BOOT("Initializing PowerMonitor...");
  powerMonitor.begin(faultHandler, ina219Driver, sdManager);
  LOG_BOOT("PowerMonitor Initialized.");

  // --- Display Live SOC on Boot Screen ---
  Adafruit_SSD1306* display3 = displayManager.getDisplay(2); // Bottom OLED
  if (display3) {
      char socStr[16];
      snprintf(socStr, sizeof(socStr), "SOC: %.1f%%", powerMonitor.getSOC());
      display3->setCursor(0, 30);
      display3->println(socStr);
      display3->display();
  }

  LOG_BOOT("Creating FreeRTOS tasks...");
  xTaskCreate(i2cTask, "i2cTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
  xTaskCreate(sensorTask, "sensorTask", 4096, NULL, TASK_PRIORITY_NORMAL, NULL); // Increased stack for filter logic
  xTaskCreate(telemetryTask, "telemetryTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
  LOG_BOOT("Initialization Complete. Handing over to RTOS.");
}

// =================================================================
// loop() - Main Task (unused in FreeRTOS)
// =================================================================
void loop() {
  vTaskSuspend(NULL);
}

// =================================================================
// RTOS Tasks
// =================================================================

/**
 * @brief FreeRTOS task for handling periodic I2C device communications.
 */
void i2cTask(void* pvParameters) {
    for (;;) {
        powerMonitor.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief FreeRTOS task for sensor data acquisition and processing.
 * --- MODIFIED: This task now reads raw ADC values and processes them through the FilterManager.
 */
void sensorTask(void* pvParameters) {
    for (;;) {
        // --- Step 1: Acquire raw data from ADCs ---
        double raw_ph_voltage = adcManager.getVoltage(0);
        double raw_ec_voltage = adcManager.getVoltage(1);

        // --- Step 2: Process raw data through the FilterManager ---
        // The global variables are updated here, making the clean data available
        // to other tasks like the telemetryTask.
        filtered_ph_voltage = filterManager.process(raw_ph_voltage);
        filtered_ec_voltage = filterManager.process(raw_ec_voltage);

        // The task now runs at a higher frequency to feed the filters more data.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief FreeRTOS task for gathering and reporting telemetry data.
 * --- MODIFIED: This task now reports the clean, filtered voltages.
 */
void telemetryTask(void* pvParameters) {
    for (;;) {
        // Gather data from all relevant managers and global variables
        float soc = powerMonitor.getSOC();
        float soh = powerMonitor.getSOH();
        bool charging = powerMonitor.isCharging();

        // Print a formatted telemetry string to the serial monitor.
        // This now shows the final, filtered values from the sensorTask.
        Serial.printf(
            "[TELEMETRY] pH_mV_filtered: %.2f, EC_mV_filtered: %.2f, SOC: %.1f%%, SOH: %.1f%%, Charging: %s\n",
            filtered_ph_voltage,
            filtered_ec_voltage,
            soc,
            soh,
            charging ? "YES" : "NO"
        );

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

#endif // PIO_UNIT_TESTING