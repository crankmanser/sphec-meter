// src/app/SensorEngine/DataPublisher.cpp
// MODIFIED FILE
#include "DataPublisher.h"
#include "DebugMacros.h"

// Include the full headers for the managers this module uses.
#include "app/TelemetrySerializer.h"
#include "app/WebService.h"
#include "managers/connectivity/MqttManager.h"
#if (ENABLE_BLE_STACK)
#include "managers/connectivity/BleManager.h"
#endif

DataPublisher::DataPublisher(
    TelemetrySerializer* serializer,
    WebService* webService,
    MqttManager* mqttManager
#if (ENABLE_BLE_STACK)
    , BleManager* bleManager
#endif
) :
    _serializer(serializer),
    _webService(webService),
    _mqttManager(mqttManager)
#if (ENABLE_BLE_STACK)
    , _bleManager(bleManager)
#endif
{}

void DataPublisher::publish() {
    // This logic is migrated directly from the old TelemetryTask.

    // 1. Generate the latest telemetry data JSON string.
    _serializer->update();
    const std::string& telemetryJson = _serializer->getSerializedTelemetry();
    LOG_MAIN("Telemetry: %s\n", telemetryJson.c_str());

    // 2. Publish the data to all active connectivity managers.
    _webService->sendTelemetry(telemetryJson);
    _mqttManager->publishTelemetry(telemetryJson);

#if (ENABLE_BLE_STACK)
    if (_bleManager && _bleManager->isClientConnected()) {
        _bleManager->setTelemetry(telemetryJson);
    }
#endif
}