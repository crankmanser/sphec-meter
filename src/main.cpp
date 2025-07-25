// File Path: /src/main.cpp

// This preprocessor guard is the key to solving the linker errors.
// The code inside this block will only be compiled when we are NOT
// running a unit test.
#ifndef PIO_UNIT_TESTING

#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"

// --- Cabinet Headers ---
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <SdManager.h> // Now included

// =================================================================
// Global Objects (Cabinet Instances)
// =================================================================
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
SdManager sdManager; // Now included

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  LOG_BOOT("SpHEC Meter v2.1.14 Booting...");

  LOG_BOOT("Initializing Core Cabinets...");
  faultHandler.begin();
  displayManager.begin(faultHandler);
  displayManager.showBootScreen();

  // Initialize the SdManager.
  // Note: The blueprint specifies that the ADC manager must be initialized before the
  // SdManager. We will add that logic when the ADC manager is created.
  if (sdManager.begin(faultHandler, SD_CS_PIN)) {
      LOG_BOOT("SdManager Initialized.");
  } else {
      LOG_BOOT("FATAL: SdManager FAILED to initialize.");
      // Here you would trigger a fault or a persistent error on the display
  }

  // Initialize higher-level cabinets that may depend on storage
  configManager.begin(faultHandler);


  LOG_BOOT("Mode: Normal");
  LOG_BOOT("Creating FreeRTOS tasks...");
  LOG_BOOT("Initialization Complete. Handing over to RTOS.");
}

// =================================================================
// loop() - Main Task (Arduino Core 1)
// =================================================================
void loop() {
  delay(1000);
}

#endif // PIO_UNIT_TESTING