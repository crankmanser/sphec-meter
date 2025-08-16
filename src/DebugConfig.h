// File Path: /src/DebugConfig.h
// MODIFIED FILE

#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

// =================================================================
// Compile-Time Debugging Configuration
// =================================================================
// This file controls the inclusion of Serial.print statements at compile time.
// Set a module's switch to 1 to enable its logs, or 0 to disable them.
// Disabled logs are completely removed by the preprocessor, saving program
// space and execution time.

// --- MASTER DEBUG SWITCHES ---
#define DEBUG_BOOT            1 // For boot sequence, mode selection, initializations
#define DEBUG_UI              1 // For UIEngine and screen-specific logs
#define DEBUG_COMM            0 // For WiFi, BLE, and API communication
#define DEBUG_SENSORS         0 // For raw and processed sensor data from managers
#define DEBUG_SPI             1 // For low-level SPI bus transactions
#define DEBUG_I2C             1 // For low-level I2C bus transactions
#define DEBUG_FILTER          1 // For the PI Filter's state and calculations
#define DEBUG_NOISE_ANALYSIS  1 // For the Noise Analysis Engine results
#define DEBUG_POWER           0 // For the PowerMonitor cabinet
#define DEBUG_STORAGE         0 // For the StorageEngine and file operations
#define DEBUG_AUTO_TUNE       1 // For the GuidedTuningEngine
#define DEBUG_FILTER_PIPELINE 0 // For tracing data flow through the HF/LF pipeline

// --- NEW: A dedicated switch for the diagnostic pipeline report ---
#define DEBUG_DIAGNOSTIC_PIPELINE 1 // Set to 1 to enable the detailed filter state report

// --- HELPER MACROS ---
// Example Usage: LOG_BOOT("Initializing %s...", "DisplayManager");

#if DEBUG_BOOT == 1
    #define LOG_BOOT(x, ...) Serial.printf("[BOOT] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_BOOT(x, ...)
#endif

#if DEBUG_FILTER == 1
    #define LOG_FILTER(x, ...) Serial.printf("[FILTER] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_FILTER(x, ...)
#endif

#if DEBUG_STORAGE == 1
    #define LOG_STORAGE(x, ...) Serial.printf("[STORAGE] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_STORAGE(x, ...)
#endif

#if DEBUG_AUTO_TUNE == 1
    #define LOG_AUTO_TUNE(x, ...) Serial.printf("[AUTOTUNE] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_AUTO_TUNE(x, ...)
#endif

#if DEBUG_FILTER_PIPELINE == 1
    #define LOG_FILTER_PIPELINE(x, ...) Serial.printf("[PIPELINE] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_FILTER_PIPELINE(x, ...)
#endif

// --- NEW: Helper macro for the diagnostic report ---
#if DEBUG_DIAGNOSTIC_PIPELINE == 1
    #define LOG_DIAG(x, ...) Serial.printf("[DIAG] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_DIAG(x, ...)
#endif


// Add other macros for other debug switches as needed...

#endif // DEBUG_CONFIG_H