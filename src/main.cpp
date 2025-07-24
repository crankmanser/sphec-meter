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
// #include <DisplayManager.h> // We will add this back after the test passes

// =================================================================
// Global Objects (Cabinet Instances)
// =================================================================
FaultHandler faultHandler;
ConfigManager configManager;
// DisplayManager displayManager;

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  LOG_BOOT("SpHEC Meter v2.1.0 Booting...");

  LOG_BOOT("Initializing Cabinets...");
  faultHandler.begin();
  configManager.begin(faultHandler);
  // displayManager.begin(faultHandler); // Add back later
  // displayManager.showBootScreen();

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