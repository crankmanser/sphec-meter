// src/boot/init_tasks.cpp
// MODIFIED FILE
#include "init_tasks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app/SensorTask.h"
#include "app/ConnectivityTask.h"
#include "app/TelemetryTask.h"
#include "app/UiTask.h"
#include "app/EncoderTask.h" // <<< ADDED: Include the new encoder task
#include "DebugMacros.h"

void init_tasks() {
    // --- Core 0: Background Processing & Connectivity ---
    xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, NULL, 1, NULL, 0);

    // --- Core 1: User Interface & High-Priority Input ---
    // The new EncoderTask runs at a higher priority than the UI to ensure
    // no input events are ever missed.
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, NULL, 3, NULL, 1);

    LOG_MAIN("All RTOS tasks created and pinned to cores.\n");
}