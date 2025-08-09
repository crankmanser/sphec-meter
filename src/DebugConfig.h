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
#define DEBUG_BOOT           1 // For boot sequence, mode selection, initializations
#define DEBUG_UI             0 // For UIEngine and screen-specific logs
#define DEBUG_COMM           0 // For WiFi, BLE, and API communication
#define DEBUG_SENSORS        0 // For raw and processed sensor data from managers
#define DEBUG_SPI            0 // For low-level SPI bus transactions
#define DEBUG_I2C            0 // For low-level I2C bus transactions
#define DEBUG_FILTER         1 // For the PI Filter's state and calculations
#define DEBUG_NOISE_ANALYSIS 0 // For the Noise Analysis Engine results
#define DEBUG_POWER          0 // For the PowerMonitor cabinet
#define DEBUG_STORAGE        1 // For the StorageEngine and file operations
#define DEBUG_AUTO_TUNE      1 // For the GuidedTuningEngine

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

/**
 * @brief --- DEFINITIVE FIX: Ensures the macro compiles to nothing when debugging is off. ---
 * This prevents the Serial.printf call from blocking execution when the serial
 * monitor is disconnected, which is the root cause of the "Heisenbug" crash.
 *
 */
#if DEBUG_AUTO_TUNE == 1
    #define LOG_AUTO_TUNE(x, ...) Serial.printf("[AUTOTUNE] " x "\n", ##__VA_ARGS__)
#else
    #define LOG_AUTO_TUNE(x, ...) // This line is critical.
#endif


// Add other macros for other debug switches as needed...

#endif // DEBUG_CONFIG_H