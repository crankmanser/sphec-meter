// src/boot/init_tasks.cpp
#include "init_tasks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app/SensorTask.h"
#include "app/ConnectivityTask.h"
#include "app/TelemetryTask.h"
#include "app/UiTask.h"
#include "DebugMacros.h"

void init_tasks() {
    xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, NULL, 3, NULL, 1);
    LOG_MAIN("All RTOS tasks created.\n");
}