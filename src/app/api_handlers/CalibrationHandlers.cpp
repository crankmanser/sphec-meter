// src/app/api_handlers/CalibrationHandlers.cpp
#include "CalibrationHandlers.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>

namespace Handlers {

void handlePhCalibrationStability(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        LOG_MAIN("[WS_ERROR] PH stability: SensorProcessor is null.\n");
        return;
    }
    float stability = sensorProcessor->getPhStability();
    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["stability_percent"] = stability;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleEcCalibrationStability(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        LOG_MAIN("[WS_ERROR] EC stability: SensorProcessor is null.\n");
        return;
    }
    float stability = sensorProcessor->getEcStability();
    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["stability_percent"] = stability;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleStartCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    sensorProcessor->startCalibration(type);
    DynamicJsonDocument doc(128);
    doc["status"] = "ok";
    doc["message"] = (type == CalibrationEngine::SensorType::PH) ? "pH calibration started." : "EC calibration started.";
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleCaptureCalibrationPoint(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    if (!request->hasParam("point_type", true) || !request->hasParam("known_value", true)) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Missing 'point_type' or 'known_value' POST parameter.\"}");
        return;
    }

    String pointTypeStr = request->getParam("point_type", true)->value();
    float knownValue = request->getParam("known_value", true)->value().toFloat();

    SensorProcessor::CalibrationPointType pointType;
    if (pointTypeStr.equalsIgnoreCase("low")) {
        pointType = SensorProcessor::CalibrationPointType::POINT_LOW;
    } else if (pointTypeStr.equalsIgnoreCase("mid")) {
        pointType = SensorProcessor::CalibrationPointType::POINT_MID;
    } else if (pointTypeStr.equalsIgnoreCase("high")) {
        pointType = SensorProcessor::CalibrationPointType::POINT_HIGH;
    } else {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid 'point_type'. Must be 'low', 'mid', or 'high'.\"}");
        return;
    }

    if (isnan(knownValue)) {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid 'known_value'. Must be a number.\"}");
        return;
    }

    if (sensorProcessor->captureCalibrationPoint(type, pointType, knownValue)) {
        DynamicJsonDocument doc(128);
        doc["status"] = "ok";
        doc["message"] = "Calibration point captured.";
        doc["next_point"] = (uint8_t)sensorProcessor->getNextCalibrationPointType(type);
        std::string response;
        serializeJson(doc, response);
        request->send(200, "application/json", response.c_str());
    } else {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Failed to capture calibration point. Is calibration active and point in sequence?\"}");
    }
}

void handleFinalizeCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    if (sensorProcessor->finalizeCalibration(type)) {
        DynamicJsonDocument doc(256);
        doc["status"] = "ok";
        doc["message"] = (type == CalibrationEngine::SensorType::PH) ? "pH calibration finalized and saved." : "EC calibration finalized and saved.";
        const CalibrationModel& model = (type == CalibrationEngine::SensorType::PH) ? sensorProcessor->getPhModel() : sensorProcessor->getEcModel();
        doc["quality_score"] = model.quality_score;
        doc["sensor_drift"] = model.last_sensor_drift;
        doc["is_valid"] = model.is_valid;
        std::string response;
        serializeJson(doc, response);
        request->send(200, "application/json", response.c_str());
    } else {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"Failed to finalize calibration. Not enough points captured?\"}");
    }
}

void handleCancelCalibration(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    sensorProcessor->cancelCalibration(type);
    DynamicJsonDocument doc(128);
    doc["status"] = "ok";
    doc["message"] = (type == CalibrationEngine::SensorType::PH) ? "pH calibration cancelled." : "EC calibration cancelled.";
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleCalibrationStatus(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor, CalibrationEngine::SensorType type) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }

    DynamicJsonDocument doc(128);
    doc["status"] = "ok";
    doc["active"] = sensorProcessor->isCalibrationActive(type);
    
    String nextPointStr = "none";
    if (sensorProcessor->isCalibrationActive(type)) {
        SensorProcessor::CalibrationPointType nextPoint = sensorProcessor->getNextCalibrationPointType(type);
        if (nextPoint == SensorProcessor::CalibrationPointType::POINT_LOW) {
            nextPointStr = "low";
        } else if (nextPoint == SensorProcessor::CalibrationPointType::POINT_MID) {
            nextPointStr = "mid";
        } else if (nextPoint == SensorProcessor::CalibrationPointType::POINT_HIGH) {
            nextPointStr = "high";
        }
    }
    doc["next_point_type"] = nextPointStr;

    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

} // namespace Handlers