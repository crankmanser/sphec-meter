#include "app/TelemetryTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "app/TelemetrySerializer.h"
#include "managers/connectivity/MqttManager.h"
#include "app/WebService.h" // <<< MODIFIED: Include WebService instead of WifiManager for telemetry

// Forward declarations of global manager pointers from main.cpp
extern TelemetrySerializer* telemetrySerializer;
extern MqttManager* mqttManager;
extern WebService* webService; // <<< MODIFIED: Point to WebService now

#if (ENABLE_BLE_STACK)
#include "managers/BleManager.h"
extern BleManager* bleManager;
#endif

// Define the delay for the telemetry task loop
const TickType_t TELEMETRY_DELAY_MS = 2000;

void telemetryTask(void* pvParameters) {
    LOG_TASK("Telemetry Task started.\n");

    for (;;) {
        // 1. Generate the latest telemetry data
        telemetrySerializer->update();
        const std::string& telemetryJson = telemetrySerializer->getSerializedTelemetry();
        LOG_MAIN("Telemetry: %s\n", telemetryJson.c_str());

        // 2. Publish the data to all active connectivity managers
        webService->sendTelemetry(telemetryJson); // <<< MODIFIED: Call WebService to send WebSocket telemetry
        mqttManager->publishTelemetry(telemetryJson);

        #if (ENABLE_BLE_STACK)
        if (bleManager->isClientConnected()) {
            bleManager->setTelemetry(telemetryJson);
        }
        #endif

        // 3. Wait for the next cycle
        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_DELAY_MS));
    }
}