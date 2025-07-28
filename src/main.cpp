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
#include <FilterManager.h>
#include <CalibrationManager.h>
#include <TempManager.h>

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
TempManager tempManager;

FilterManager phFilter;
FilterManager ecFilter;
FilterManager v3_3_Filter;
FilterManager v5_0_Filter;

CalibrationManager phCalManager;
CalibrationManager ecCalManager;

SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
portMUX_TYPE sharedDataMux = portMUX_INITIALIZER_UNLOCKED;

// --- Global variables to hold the full data pipeline ---
double final_ph_value = 0.0;
double final_ec_value = 0.0;
double final_probe_temp = 0.0;
double final_ambient_temp = 0.0;
double final_humidity = 0.0;
double final_v3_3_bus = 0.0;
double final_v5_0_bus = 0.0;

// --- Debugging globals for telemetry ---
volatile double dbg_raw_ph_v = 0.0;
volatile double dbg_filtered_ph_v = 0.0;
volatile double dbg_raw_ec_v = 0.0;
volatile double dbg_filtered_ec_v = 0.0;


// =================================================================
// FreeRTOS Task Prototypes & Helper Functions
// =================================================================
void i2cTask(void* pvParameters);
void oneWireTask(void* pvParameters);
void sensorTask(void* pvParameters);
void telemetryTask(void* pvParameters);
void serialCalTask(void* pvParameters);
String getSerialInput();

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    LOG_BOOT("SpHEC Meter v2.8.4 Booting...");

    spiMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    faultHandler.begin();
    displayManager.begin(faultHandler);
    displayManager.showBootScreen();
    adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    configManager.begin(faultHandler);
    
    phFilter.begin(faultHandler, configManager);
    ecFilter.begin(faultHandler, configManager);
    v3_3_Filter.begin(faultHandler, configManager);
    v5_0_Filter.begin(faultHandler, configManager);
    phCalManager.begin(faultHandler);
    ecCalManager.begin(faultHandler);
    tempManager.begin(faultHandler);
    ina219Driver.begin(faultHandler);
    powerMonitor.begin(faultHandler, ina219Driver, sdManager);

    StaticJsonDocument<512> phCalDoc;
    if (sdManager.loadJson("/ph_cal.json", phCalDoc)) {
        phCalManager.deserializeModel(phCalManager.getMutableCurrentModel(), phCalDoc);
    }
    StaticJsonDocument<512> ecCalDoc;
    if (sdManager.loadJson("/ec_cal.json", ecCalDoc)) {
        ecCalManager.deserializeModel(ecCalManager.getMutableCurrentModel(), ecCalDoc);
    }
    
    LOG_BOOT("Creating FreeRTOS tasks...");
    xTaskCreate(i2cTask, "i2cTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(sensorTask, "sensorTask", 4096, NULL, TASK_PRIORITY_NORMAL, NULL);
    xTaskCreate(telemetryTask, "telemetryTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
    xTaskCreate(serialCalTask, "serialCalTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
    LOG_BOOT("Initialization Complete.");

    adcManager.setProbeState(0, ProbeState::ACTIVE);
    adcManager.setProbeState(1, ProbeState::ACTIVE);
}

void loop() { vTaskSuspend(NULL); }

// =================================================================
// RTOS Tasks & Helper Functions
// =================================================================
String getSerialInput() {
    String input = "";
    char c;
    while (true) {
        if (Serial.available()) {
            c = Serial.read();
            if (c == '\n' || c == '\r') {
                while(Serial.available() && (Serial.peek() == '\n' || Serial.peek() == '\r')) { Serial.read(); }
                return input;
            }
            input += c;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void i2cTask(void* pvParameters) {
    for (;;) {
        powerMonitor.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void oneWireTask(void* pvParameters) {
    for (;;) {
        tempManager.update();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief FreeRTOS task for the full sensor data processing pipeline.
 * --- MODIFIED: Refactored logic to be cleaner and added debug prints.
 */
void sensorTask(void* pvParameters) {
    for (;;) {
        // --- Get latest temperature data first ---
        final_probe_temp = tempManager.getProbeTemp();
        final_ambient_temp = tempManager.getAmbientTemp();
        final_humidity = tempManager.getHumidity();

        // --- Process pH Probe Pipeline ---
        double raw_ph_v = adcManager.getVoltage(0, ADS1118::DIFF_0_1);
        double filtered_ph_v = phFilter.process(raw_ph_v);
        double calibrated_ph = phCalManager.getCalibratedValue(filtered_ph_v);
        final_ph_value = phCalManager.getCompensatedValue(calibrated_ph, final_probe_temp, false);
        
        // --- Process EC Probe Pipeline ---
        double raw_ec_v = adcManager.getVoltage(1, ADS1118::DIFF_0_1);
        double filtered_ec_v = ecFilter.process(raw_ec_v);
        double calibrated_ec = ecCalManager.getCalibratedValue(filtered_ec_v);
        final_ec_value = ecCalManager.getCompensatedValue(calibrated_ec, final_probe_temp, true);

        // --- Process Bus Voltages ---
        double raw_v3_3 = adcManager.getVoltage(0, ADS1118::AIN_2); 
        final_v3_3_bus = v3_3_Filter.process(raw_v3_3);

        double raw_v5_0 = adcManager.getVoltage(1, ADS1118::AIN_2);
        final_v5_0_bus = v5_0_Filter.process(raw_v5_0);

        // --- Update debugging globals for telemetry ---
        taskENTER_CRITICAL(&sharedDataMux);
        dbg_raw_ph_v = raw_ph_v;
        dbg_filtered_ph_v = filtered_ph_v;
        dbg_raw_ec_v = raw_ec_v;
        dbg_filtered_ec_v = filtered_ec_v;
        taskEXIT_CRITICAL(&sharedDataMux);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void telemetryTask(void* pvParameters) {
    for (;;) {
        double pH, ec, p_temp, a_temp, hum, v33, v50;
        double r_ph, f_ph, r_ec, f_ec;
        
        taskENTER_CRITICAL(&sharedDataMux);
        pH = final_ph_value; ec = final_ec_value; p_temp = final_probe_temp; a_temp = final_ambient_temp;
        hum = final_humidity; v33 = final_v3_3_bus; v50 = final_v5_0_bus;
        r_ph = dbg_raw_ph_v; f_ph = dbg_filtered_ph_v; r_ec = dbg_raw_ec_v; f_ec = dbg_filtered_ec_v;
        taskEXIT_CRITICAL(&sharedDataMux);
        
        Serial.println("\n[TELEMETRY] -----------------------------");
        Serial.printf("  Probes:\n");
        Serial.printf("    - pH:         %.2f (Raw: %.2f mV, Filtered: %.2f mV)\n", pH, r_ph, f_ph);
        Serial.printf("    - EC:         %.2f uS/cm (Raw: %.2f mV, Filtered: %.2f mV)\n", ec, r_ec, f_ec);
        Serial.printf("  Temperatures:\n");
        Serial.printf("    - Probe:      %.2f C\n", p_temp);
        Serial.printf("    - Ambient:    %.2f C\n", a_temp);
        Serial.printf("    - Humidity:   %.1f %%\n", hum);
        Serial.printf("  System Bus:\n");
        Serial.printf("    - 3.3V Bus:   %.2f mV\n", v33);
        Serial.printf("    - 5.0V Bus:   %.2f mV\n", v50);
        Serial.printf("  Power:\n");
        Serial.printf("    - SOC:        %.1f %%\n", powerMonitor.getSOC());
        Serial.printf("    - Current:    %.2f mA\n", powerMonitor.getCurrent());
        Serial.println("-----------------------------------------\n");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void serialCalTask(void* pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    Serial.println("\n\n===== Serial Calibration Wizard =====");
    Serial.println("Press 'p' to start 3-point pH calibration.");
    Serial.println("Any other key to skip.");

    String choice = getSerialInput();

    if (choice == "p") {
        Serial.println("Starting pH Calibration...");
        phCalManager.startNewCalibration();
        const CalibrationModel& previousPhModel = phCalManager.getCurrentModel();
        for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) {
            Serial.printf("\n--- Point %d ---\n", i + 1);
            Serial.print("Enter nominal pH value for this point (e.g., 4.01): ");
            String input = getSerialInput();
            double nominalValue = input.toFloat();

            Serial.printf("Place probe in pH %.2f buffer solution.\n", nominalValue);
            Serial.println("Press ENTER when ready to capture voltage...");
            getSerialInput();
            
            double capturedVoltage, capturedTemp;
            taskENTER_CRITICAL(&sharedDataMux);
            capturedVoltage = dbg_filtered_ph_v; // Use the debug variable to ensure it's the latest
            capturedTemp = final_probe_temp;
            taskEXIT_CRITICAL(&sharedDataMux);
            
            phCalManager.addCalibrationPoint(capturedVoltage, nominalValue, capturedTemp);
            Serial.printf("Captured Point %d: %.2f mV @ %.2f C -> Nominal pH %.2f\n", i + 1, capturedVoltage, capturedTemp, nominalValue);
        }
        Serial.println("\nCalculating new model...");
        double quality = phCalManager.calculateNewModel(previousPhModel);
        if (quality > 0) {
            Serial.printf("Calculation complete. Quality Score: %.1f%%\n", quality);
            phCalManager.acceptNewModel();
            StaticJsonDocument<512> docToSave;
            phCalManager.serializeModel(phCalManager.getCurrentModel(), docToSave);
            if (sdManager.saveJson("/ph_cal.json", docToSave)) {
                Serial.println("SUCCESS: New pH model saved. Please reboot.");
            } else {
                Serial.println("ERROR: Failed to save model to SD card.");
            }
        } else {
            Serial.println("ERROR: Failed to calculate new model.");
        }
    } else {
        Serial.println("Skipping calibration.");
    }
    Serial.println("Calibration task finished. Suspending task.");
    vTaskSuspend(NULL);
}
#endif // PIO_UNIT_TESTING