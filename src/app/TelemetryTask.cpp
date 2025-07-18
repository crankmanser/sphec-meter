// src/app/TelemetryTask.cpp
// MODIFIED FILE
#include "app/TelemetryTask.h"
#include "app/AppContext.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "app/TelemetrySerializer.h"
#include "managers/connectivity/MqttManager.h"
#include "app/WebService.h"

#if (ENABLE_BLE_STACK)
#include "managers/connectivity/BleManager.h"
#endif

const TickType_t TELEMETRY_DELAY_MS = 2000;

void telemetryTask(void* pvParameters) {
    LOG_TASK("Telemetry Task started.\n");
    AppContext* context = static_cast<AppContext*>(pvParameters);

    for (;;) {
        context->telemetrySerializer->update();
        const std::string& telemetryJson = context->telemetrySerializer->getSerializedTelemetry();
        LOG_MAIN("Telemetry: %s\n", telemetryJson.c_str());

        context->webService->sendTelemetry(telemetryJson);
        context->mqttManager->publishTelemetry(telemetryJson);

        #if (ENABLE_BLE_STACK)
        if (context->bleManager && context->bleManager->isClientConnected()) {
            context->bleManager->setTelemetry(telemetryJson);
        }
        #endif

        vTaskDelay(pdMS_TO_TICKS(TELEMETRY_DELAY_MS));
    }
}