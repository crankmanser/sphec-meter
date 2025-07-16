// src/boot/init_tasks.cpp
// MODIFIED FILE
#include "init_tasks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app/SensorTask.h"
#include "app/ConnectivityTask.h"
#include "app/TelemetryTask.h"
#include "app/UiTask.h"
#include "app/EncoderTask.h"
#include "app/common/SystemState.h" 
#include "DebugMacros.h"

// External declarations for RTOS task handles
extern TaskHandle_t g_sensorTaskHandle;
extern TaskHandle_t g_telemetryTaskHandle;
extern TaskHandle_t g_connectivityTaskHandle;

void init_tasks() {
    // --- FIX: Implement the new, robust priority scheme ---
    
    // Create tasks that run in ALL modes with their new priorities.
    // Connectivity is a low-priority housekeeping task.
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, NULL, 1, &g_connectivityTaskHandle, 0);
    // Encoder is high-priority to ensure UI responsiveness.
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, NULL, 4, NULL, 1);
    
    // Create tasks that ONLY run in NORMAL mode
    if (g_boot_mode == BootMode::NORMAL) {
        LOG_MAIN("Boot Mode: NORMAL. Creating full task set.\n");
        // Telemetry and Sensor tasks are background data processors.
        xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, NULL, 2, &g_telemetryTaskHandle, 0);
        xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, &g_sensorTaskHandle, 0);
    } else {
        LOG_MAIN("Boot Mode: DIAGNOSTICS. Skipping non-essential tasks.\n");
    }

    LOG_MAIN("RTOS task creation complete.\n");
}
