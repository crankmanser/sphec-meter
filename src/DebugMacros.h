// src/DebugMacros.h
// MODIFIED FILE
#pragma once

#include "config/DebugConfig.h"
#include <cstdio>
#include <Arduino.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if (MASTER_DEBUG_ENABLE)
    #define LOG_INIT() do { Serial.begin(115200); while(!Serial); } while(0)
    #define LOG_PREPEND() printf("[%lu][%s] ", millis(), pcTaskGetName(NULL))
#else
    #define LOG_INIT()
    #define LOG_PREPEND()
#endif

// <<< NEW: Dedicated logger for the boot sequence >>>
#if (MASTER_DEBUG_ENABLE && ENABLE_DEBUG_MAIN)
    #define LOG_BOOT(format, ...) do { LOG_PREPEND(); printf("[BOOT] " format, ##__VA_ARGS__); } while(0)
#else
    #define LOG_BOOT(...)
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
