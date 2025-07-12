// src/app/WebService.h
#pragma once

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <string>
#include "managers/storage/StorageManager.h"
#include "config/Network_Config.h"
#include "helpers/Base64.h"
#include <ArduinoJson.h>
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/RawSensorReader.h"
#include "managers/sensor/calibration/CalibrationSessionManager.h" 

class WebService {
public:
    WebService(StorageManager& storage, NetworkConfig& config, SensorProcessor* sensorProcessor, RawSensorReader* rawSensorReader);
    void begin();
    void sendTelemetry(const std::string& json);

private:
    StorageManager& _storage;
    NetworkConfig& _config;
    SensorProcessor* _sensorProcessor;
    RawSensorReader* _rawSensorReader; 
    AsyncWebServer _server;
    AsyncWebSocket _ws;

    // <<< MODIFIED: All private handlers are now removed. This class is a pure coordinator. >>>

    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t* data, size_t len);
};