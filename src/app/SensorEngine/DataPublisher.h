// src/app/SensorEngine/DataPublisher.h
// MODIFIED FILE
#pragma once

#include "config/DebugConfig.h" // For ENABLE_BLE_STACK

// Forward-declare dependencies
class TelemetrySerializer;
class WebService;
class MqttManager;
#if (ENABLE_BLE_STACK)
class BleManager;
#endif

/**
 * @class DataPublisher
 * @brief A helper module responsible for publishing data to all connectivity endpoints.
 *
 * This class orchestrates the various connectivity managers to send the
 * telemetry data. It is designed to be called from the master SensorEngineTask.
 */
class DataPublisher {
public:
    /**
     * @brief Constructs the DataPublisher.
     * @param serializer Pointer to the telemetry JSON serializer.
     * @param webService Pointer to the web service for WebSocket publishing.
     * @param mqttManager Pointer to the MQTT client manager.
     * @param bleManager Pointer to the BLE manager.
     */
    DataPublisher(
        TelemetrySerializer* serializer,
        WebService* webService,
        MqttManager* mqttManager
#if (ENABLE_BLE_STACK)
        , BleManager* bleManager
#endif
    );

    /**
     * @brief Executes one full cycle of data publication.
     *
     * This method serializes the latest processed data and sends it to all
     * active connectivity endpoints (WebSocket, MQTT, BLE).
     */
    void publish();

private:
    TelemetrySerializer* _serializer;
    WebService* _webService;
    MqttManager* _mqttManager;
#if (ENABLE_BLE_STACK)
    BleManager* _bleManager;
#endif
};