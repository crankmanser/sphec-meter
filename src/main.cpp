// File Path: /src/main.cpp

#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"

// =================================================================
// Global Objects (Cabinet Instances)
// =================================================================
// Placeholder for global manager instances.
// e.g., DisplayManager displayManager;
//       ConfigManager configManager;

// =================================================================
// setup() - Application Entry Point
// =================================================================
void setup() {
  // 1. Initialize Serial Communication for debugging
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to connect
  LOG_BOOT("SpHEC Meter v2.0.0 Booting...");

  // 2. Initialize Core Systems & Managers
  // This is where we will initialize all our "Cabinet" objects.
  // The order is critical for some components.
  LOG_BOOT("Initializing Cabinets...");
  // e.g., configManager.begin();
  //      faultHandler.begin();
  //      displayManager.begin(); // Must be early for boot screen

  // 3. Present Boot Menu (Normal vs. Diagnostics)
  // Logic to read user input and decide the boot mode will go here.
  // For now, we will default to Normal Mode.
  LOG_BOOT("Mode: Normal");

  // 4. Initialize Hardware-Specific Managers based on mode
  // The SPI Bus Precedence Rule MUST be followed here.
  // a. Initialize ADS_Manager (ADC) to "prime" the bus.
  // b. Initialize SdManager (SD Card).

  // 5. Load all critical configurations from storage
  // e.g., storageEngine.loadAllCriticalConfigs();

  // 6. Create FreeRTOS Tasks
  // All ongoing logic will be handled by tasks, not the loop().
  LOG_BOOT("Creating FreeRTOS tasks...");
  // e.g., xTaskCreatePinnedToCore(uiTask, "UITask", 8192, NULL, TASK_PRIORITY_HIGH, NULL, 1);
  //      xTaskCreatePinnedToCore(sdTask, "SDTask", 4096, NULL, SD_TASK_PRIORITY_NORMAL, NULL, 0);
  
  LOG_BOOT("Initialization Complete. Handing over to RTOS.");
}

// =================================================================
// loop() - Main Task (Arduino Core 1)
// =================================================================
void loop() {
  // The loop() function is effectively the main task for Core 1.
  // In a FreeRTOS environment, this loop should remain empty or be used
  // for very specific, non-blocking tasks. All our logic is encapsulated
  // in the tasks created in setup().
  delay(1000);
}