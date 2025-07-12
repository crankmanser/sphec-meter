// src/app/api_handlers/SensorDataHandlers.cpp
#include "SensorDataHandlers.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>

namespace Handlers {

void handlePhLiveVoltage(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }
    float voltage = sensorProcessor->getFilteredPhVoltage();
    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["voltage"] = voltage;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleEcLiveVoltage(AsyncWebServerRequest *request, SensorProcessor* sensorProcessor) {
    if (!sensorProcessor) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"SensorProcessor not initialized.\"}");
        return;
    }
    float voltage = sensorProcessor->getFilteredEcVoltage();
    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["voltage"] = voltage;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handlePhRawVoltage(AsyncWebServerRequest *request, RawSensorData* rawData, SemaphoreHandle_t mutex) {
    if (!mutex) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Raw data mutex not initialized.\"}");
        return;
    }

    float raw_adc_value = 0.0f;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        raw_adc_value = rawData->adc_ph_raw;
        xSemaphoreGive(mutex);
    } else {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Could not acquire raw data mutex.\"}");
        return;
    }
    
    const float ADC_FSR_PROBES = 4.096f;
    float converted_voltage = (static_cast<float>(raw_adc_value) / 32767.0f) * ADC_FSR_PROBES;

    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["raw_voltage"] = converted_voltage;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleEcRawVoltage(AsyncWebServerRequest *request, RawSensorData* rawData, SemaphoreHandle_t mutex) {
    if (!mutex) {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Raw data mutex not initialized.\"}");
        return;
    }

    float raw_adc_value = 0.0f;
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        raw_adc_value = rawData->adc_ec_raw;
        xSemaphoreGive(mutex);
    } else {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Could not acquire raw data mutex.\"}");
        return;
    }
    
    const float ADC_FSR_PROBES = 4.096f;
    float converted_voltage = (static_cast<float>(raw_adc_value) / 32767.0f) * ADC_FSR_PROBES;

    DynamicJsonDocument doc(64);
    doc["status"] = "ok";
    doc["raw_voltage"] = converted_voltage;
    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

} // namespace Handlers