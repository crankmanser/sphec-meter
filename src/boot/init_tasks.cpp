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
#include "DebugMacros.h"

// <<< ADDED: Extern declarations for the global task handles >>>
extern TaskHandle_t g_sensorTaskHandle;
extern TaskHandle_t g_telemetryTaskHandle;
extern TaskHandle_t g_connectivityTaskHandle;

void init_tasks() {
    // --- Core 0: Background Processing & Connectivity ---
    // <<< MODIFIED: Capture the task handles upon creation >>>
    xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, NULL, 2, &g_telemetryTaskHandle, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, &g_sensorTaskHandle, 0);
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, NULL, 1, &g_connectivityTaskHandle, 0);

    // --- Core 1: User Interface & High-Priority Input ---
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, NULL, 3, NULL, 1);

    LOG_MAIN("All RTOS tasks created and pinned to cores.\n");
}