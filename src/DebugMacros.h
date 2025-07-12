#pragma once
#include "config/DebugConfig.h"
#include <cstdio>
#include "freertos/FreeRTOS.h" // Required for xTaskGetTickCount
#include "freertos/task.h"    // Required for pcTaskGetName

// This macro system allows for conditional compilation of debug messages.
// If a debug flag in DebugConfig.h is 0, the corresponding log messages
// will be completely removed by the compiler, saving memory and CPU cycles.

#if (MASTER_DEBUG_ENABLE)
    #define LOG_INIT() do { Serial.begin(115200); while(!Serial); } while(0)
    // Helper macro to get current time and task name
    #define LOG_PREPEND() printf("[%lu][%s] ", millis(), pcTaskGetName(NULL))
#else
    #define LOG_INIT()
    #define LOG_PREPEND()
#endif

#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_MAIN)
    #define LOG_MAIN(format, ...) do { LOG_PREPEND(); printf("[MAIN] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_MAIN(...)
#endif

#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_HAL)
    #define LOG_HAL(format, ...) do { LOG_PREPEND(); printf("[HAL] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_HAL(...)
#endif

#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_MANAGERS)
    #define LOG_MANAGER(format, ...) do { LOG_PREPEND(); printf("[MGR] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_MANAGER(...)
#endif

#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_TASKS)
    #define LOG_TASK(format, ...) do { LOG_PREPEND(); printf("[TASK] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_TASK(...)
#endif

#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_UI)
    #define LOG_UI(format, ...) do { LOG_PREPEND(); printf("[UI] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_UI(...)
#endif