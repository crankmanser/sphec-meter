// src/app/api_handlers/TuningHandlers.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "managers/sensor/SensorProcessor.h"

namespace Handlers {
    void handlePhFilterTuningGet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handleEcFilterTuningGet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handlePhFilterTuningSet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handleEcFilterTuningSet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
}