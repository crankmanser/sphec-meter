// src/app/common/SystemState.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h" // Required for SemaphoreHandle_t
#include "freertos/semphr.h"   // Required for SemaphoreHandle_t

// Defines the boot modes for the entire system.
enum class BootMode {
    NORMAL,
    DIAGNOSTICS
};

// A global variable to hold the detected boot mode.
// It is defined in main.cpp and declared here as 'extern' for access by other files.
extern BootMode g_boot_mode;

// <<< ADDED: Extern declarations for global data mutexes >>>
// These variables are defined in main.cpp. Declaring them here allows
// any other file that includes this header to safely access them.
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;