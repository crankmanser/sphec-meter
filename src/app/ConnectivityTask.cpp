#include "app/ConnectivityTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "managers/connectivity/WifiManager.h"
#include "managers/connectivity/MqttManager.h"

// Forward declarations of global manager pointers from main.cpp
extern WifiManager* wifiManager;
extern MqttManager* mqttManager;

// Define the delay for the connectivity task loop
const TickType_t CONNECTIVITY_DELAY_MS = 100;

void connectivityTask(void* pvParameters) {
    LOG_TASK("Connectivity Task started.\n");

    for (;;) {
        // Call the update methods for the managers that need periodic checks.
        wifiManager->update();
        mqttManager->update();

        // This task runs more frequently to ensure the connectivity
        // managers are responsive.
        vTaskDelay(pdMS_TO_TICKS(CONNECTIVITY_DELAY_MS));
    }
}