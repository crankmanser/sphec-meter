#pragma once

// =================================================================
// Master Debug Switch
// =================================================================
// Set this to 0 to disable all serial logging entirely.
#define MASTER_DEBUG_ENABLE 1

// =================================================================
// <<< NEW: Sensor Simulation Switch >>>
// =================================================================
// Set this to 1 to bypass real sensors and use simulated data for testing.
 // Set to 0 for normal operation.
 #define ENABLE_SENSOR_SIMULATION 0
 
// =================================================================
// <<< NEW: BLE Radio Stack Switch >>>
// =================================================================
// Set this to 0 to completely disable the BLE stack to isolate Wi-Fi.
 #define ENABLE_BLE_STACK 0

 // =================================================================
 // Layer-Specific Debug Switches
 // =================================================================

// Set these to 1 to enable logging for a specific architectural layer,
// or 0 to disable it. MASTER_DEBUG_ENABLE must be 1.
#define ENABLE_DEBUG_MAIN       1 // For main.cpp setup and loop
#define ENABLE_DEBUG_HAL        1 // For Hardware Abstraction Layer (drivers)
#define ENABLE_DEBUG_MANAGERS   1 // For Manager Layer (logic)
#define ENABLE_DEBUG_TASKS      1 // For RTOS Tasks
#define ENABLE_DEBUG_UI         1 // For UI Screens and Rendering