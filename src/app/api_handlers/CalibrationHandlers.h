// src/app/api_handlers/CalibrationHandlers.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "managers/sensor/SensorProcessor.h"

namespace Handlers {
    void handlePhCalibrationStability(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    void handleEcCalibrationStability(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor);
    
    void handleStartCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type);
    void handleCaptureCalibrationPoint(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type);
    void handleFinalizeCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type);
    void handleCancelCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type);
    void handleCalibrationStatus(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type);
}