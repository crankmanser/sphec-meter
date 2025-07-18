// src/boot/init_tasks.cpp
// MODIFIED FILE
#include "init_tasks.h"
#include "app/AppContext.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Include the real task headers
#include "app/SensorTask.h"
#include "app/ConnectivityTask.h"
#include "app/TelemetryTask.h"
#include "app/UiTask.h"
#include "app/EncoderTask.h"
#include "app/common/SystemState.h" 
#include "DebugMacros.h"

void init_tasks(AppContext* appContext) {
    LOG_MAIN("Creating RTOS tasks...\n");

    // Create tasks that run in ALL modes
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, appContext, 1, nullptr, 0);
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, appContext, 4, nullptr, 1);
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, appContext, 3, nullptr, 1);
    
    // Create tasks that ONLY run in NORMAL mode
    if (g_boot_mode == BootMode::NORMAL) {
        LOG_MAIN("Boot Mode: NORMAL. Creating full task set.\n");
        xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, appContext, 2, nullptr, 0);
        xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, appContext, 2, nullptr, 0);
    } else {
        LOG_MAIN("Boot Mode: DIAGNOSTICS. Skipping non-essential tasks.\n");
    }

    LOG_MAIN("RTOS task creation complete.\n");
}