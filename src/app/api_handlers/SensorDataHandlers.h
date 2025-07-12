// src/app/api_handlers/SensorDataHandlers.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "managers/sensor/SensorProcessor.h"
#include "data_models/SensorData_types.h"
#include "freertos/semphr.h"

namespace Handlers {
    void handlePhLiveVoltage(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handleEcLiveVoltage(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handlePhRawVoltage(AsyncWebServerRequest *request, RawSensorData* rawData, SemaphoreHandle_t mutex);
    void handleEcRawVoltage(AsyncWebServerRequest *request, RawSensorData* rawData, SemaphoreHandle_t mutex);
}