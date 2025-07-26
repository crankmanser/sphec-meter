// File Path: /src/main.cpp

#ifndef PIO_UNIT_TESTING

#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// --- Cabinet Headers ---
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <AdcManager.h>
#include <SdManager.h>

// =================================================================
// Global Objects
// =================================================================
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;

SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  LOG_BOOT("SpHEC Meter v2.1.28 Final Build...");

  spiMutex = xSemaphoreCreateMutex();
  if (spiMutex == NULL) {
      LOG_BOOT("FATAL: Failed to create SPI mutex.");
      while(1);
  }

  LOG_BOOT("Initializing shared VSPI bus...");
  vspi = new SPIClass(VSPI);
  vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
  LOG_BOOT("VSPI bus Initialized.");

  LOG_BOOT("Initializing Core Cabinets...");
  faultHandler.begin();
  
  LOG_BOOT("Initializing DisplayManager...");
  displayManager.begin(faultHandler);
  displayManager.showBootScreen();

  // --- FIX: Pass SD_CS_PIN to AdcManager ---
  LOG_BOOT("Initializing AdcManager...");
  if (adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN)) {
      LOG_BOOT("AdcManager Initialized.");
  } else {
      LOG_BOOT("FATAL: AdcManager FAILED to initialize.");
  }

  LOG_BOOT("Priming SPI bus for SdManager...");
  adcManager.getVoltage(0);
  LOG_BOOT("SPI bus primed.");

  // --- FIX: Pass ADC CS pins to SdManager ---
  LOG_BOOT("Initializing SdManager...");
  if (sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN)) {
      LOG_BOOT("SdManager Initialized SUCCESSFULLY.");
  } else {
      LOG_BOOT("FATAL: SdManager FAILED to initialize.");
  }

  LOG_BOOT("Initializing ConfigManager...");
  configManager.begin(faultHandler);
  LOG_BOOT("ConfigManager Initialized.");

  LOG_BOOT("Mode: Hardware Test");
  LOG_BOOT("Creating FreeRTOS tasks...");
  LOG_BOOT("Initialization Complete. Handing over to RTOS.");
}

// =================================================================
// loop() - Main Task
// =================================================================
void loop() {
  double voltage1 = adcManager.getVoltage(0);
  double voltage2 = adcManager.getVoltage(1);
  Serial.printf("ADC1: %.2f mV, ADC2: %.2f mV\n", voltage1, voltage2);
  delay(1000);
}

#endif // PIO_UNIT_TESTING