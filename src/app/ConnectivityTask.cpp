// src/app/ConnectivityTask.cpp
// MODIFIED FILE
#include "app/ConnectivityTask.h"
#include "app/AppContext.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "managers/connectivity/WifiManager.h"
#include "managers/connectivity/MqttManager.h"

const TickType_t CONNECTIVITY_DELAY_MS = 100;

void connectivityTask(void* pvParameters) {
    LOG_TASK("Connectivity Task started.\n");
    AppContext* context = static_cast<AppContext*>(pvParameters);

    for (;;) {
        context->wifiManager->update();
        context->mqttManager->update();
        vTaskDelay(pdMS_TO_TICKS(CONNECTIVITY_DELAY_MS));
    }
}