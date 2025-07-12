// src/app/api_handlers/TuningHandlers.cpp
#include "TuningHandlers.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>

namespace Handlers {

// Helper function to parse tuning parameters from a request
bool parseFilterTuningParams(AsyncWebServerRequest *request, PIFilter::Tuni_t& tuning) {
    bool success = true;
    if (request->hasParam("settle_threshold", true)) {
        tuning.settle_threshold = request->getParam("settle_threshold", true)->value().toFloat();
    } else { success = false; }

    if (request->hasParam("lock_smoothing", true)) {
        tuning.lock_smoothing = request->getParam("lock_smoothing", true)->value().toFloat();
    } else { success = false; }

    if (request->hasParam("track_response", true)) {
        tuning.track_response = request->getParam("track_response", true)->value().toFloat();
    } else { success = false; }

    if (request->hasParam("track_assist", true)) {
        tuning.track_assist = request->getParam("track_assist", true)->value().toFloat();
    } else { success = false; }
    
    // Add validation checks
    if (isnan(tuning.settle_threshold) || tuning.settle_threshold < 0) success = false;
    if (isnan(tuning.lock_smoothing) || tuning.lock_smoothing < 0 || tuning.lock_smoothing > 1.0f) success = false;
    if (isnan(tuning.track_response) || tuning.track_response < 0 || tuning.track_response > 1.0f) success = false;
    if (isnan(tuning.track_assist) || tuning.track_assist < 0) success = false;

    return success;
}


void handlePhFilterTuningGet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }
    const PIFilter::Tuni_t& tuning = sensorProcessor->getPhFilterTuning();
    DynamicJsonDocument doc(256);
    doc["status"] = "ok";
    doc["settle_threshold"] = tuning.settle_threshold;
    doc["lock_smoothing"] = tuning.lock_smoothing;
    doc["track_response"] = tuning.track_response;
    doc["track_assist"] = tuning.track_assist;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleEcFilterTuningGet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }
    const PIFilter::Tuni_t& tuning = sensorProcessor->getEcFilterTuning();
    DynamicJsonDocument doc(256);
    doc["status"] = "ok";
    doc["settle_threshold"] = tuning.settle_threshold;
    doc["lock_smoothing"] = tuning.lock_smoothing;
    doc["track_response"] = tuning.track_response;
    doc["track_assist"] = tuning.track_assist;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handlePhFilterTuningSet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    PIFilter::Tuni_t newTuning;
    if (!parseFilterTuningParams(request, newTuning)) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Missing or invalid filter tuning POST parameters.\"}");
        return;
    }

    sensorProcessor->setPhFilterTuning(newTuning);
    
    DynamicJsonDocument doc(128);
    doc["status"] = "ok";
    doc["message"] = "pH filter tuning updated.";
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleEcFilterTuningSet(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    PIFilter::Tuni_t newTuning;
    if (!parseFilterTuningParams(request, newTuning)) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Missing or invalid filter tuning POST parameters.\"}");
        return;
    }

    sensorProcessor->setEcFilterTuning(newTuning);
    
    DynamicJsonDocument doc(128);
    doc["status"] = "ok";
    doc["message"] = "EC filter tuning updated.";
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

} // namespace Handlers