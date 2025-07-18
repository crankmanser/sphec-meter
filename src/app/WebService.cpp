// src/app/WebService.cpp
// MODIFIED FILE
#include "app/WebService.h"
#include "DebugMacros.h"
#include <ESPmDNS.h>
#include <algorithm>
#include <ArduinoJson.h>
#include "config/calibration_config.h"

// <<< MODIFIED: Include the single source of truth for all global variables >>>
#include "app/globals.h"

// Include all handler modules
#include "api_handlers/StorageHandlers.h" 
#include "api_handlers/CalibrationHandlers.h"
#include "api_handlers/TuningHandlers.h"
#include "api_handlers/SensorDataHandlers.h"

// <<< REMOVED: Redundant extern declarations are no longer needed. >>>
// The globals are now correctly declared in globals.h

static WebService* _webServiceInstance = nullptr;

WebService::WebService(StorageManager& storage, NetworkConfig& config, SensorProcessor* sensorProcessor, RawSensorReader* rawSensorReader) :
    _storage(storage),
    _config(config),
    _sensorProcessor(sensorProcessor),
    _rawSensorReader(rawSensorReader), 
    _server(80),
    _ws("/ws")
{
    _webServiceInstance = this;
}

void WebService::begin() {
    if(MDNS.begin("sphec-meter")) {
        LOG_MANAGER("mDNS responder started\n");
    }

    // --- Storage Handlers ---
    _server.on("/api/v1/storage/diagnostics/start", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleStorageDiagnosticsStart(request, &this->_storage); });
    _server.on("/api/v1/storage/diagnostics/result", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleStorageDiagnosticsResult(request, &this->_storage); });
    _server.on("/api/v1/config/set", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleSetConfig(request, &this->_storage, &this->_config); });
    _server.on("/api/v1/backup", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleBackup(request, &this->_storage); });
    _server.on("/api/v1/restore", HTTP_POST, 
        [](AsyncWebServerRequest *request){ request->send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Restore complete, rebooting...\"}"); delay(1000); ESP.restart(); }, 
        [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){ Handlers::handleRestoreUpload(request, filename, index, data, len, final, &this->_storage); }
    );

    // --- Calibration Handlers ---
    _server.on("/api/v1/calibration/ph/stability", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handlePhCalibrationStability(request, this->_sensorProcessor); });
    _server.on("/api/v1/calibration/ec/stability", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleEcCalibrationStability(request, this->_sensorProcessor); });
    _server.on("/api/v1/calibration/ph/start", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleStartCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::PH); });
    _server.on("/api/v1/calibration/ec/start", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleStartCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::EC); });
    _server.on("/api/v1/calibration/ph/capturePoint", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleCaptureCalibrationPoint(request, this->_sensorProcessor, CalibrationEngine::SensorType::PH); });
    _server.on("/api/v1/calibration/ec/capturePoint", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleCaptureCalibrationPoint(request, this->_sensorProcessor, CalibrationEngine::SensorType::EC); });
    _server.on("/api/v1/calibration/ph/finalize", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleFinalizeCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::PH); });
    _server.on("/api/v1/calibration/ec/finalize", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleFinalizeCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::EC); });
    _server.on("/api/v1/calibration/ph/cancel", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleCancelCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::PH); });
    _server.on("/api/v1/calibration/ec/cancel", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleCancelCalibration(request, this->_sensorProcessor, CalibrationEngine::SensorType::EC); });
    _server.on("/api/v1/calibration/ph/status", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleCalibrationStatus(request, this->_sensorProcessor, CalibrationEngine::SensorType::PH); });
    _server.on("/api/v1/calibration/ec/status", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleCalibrationStatus(request, this->_sensorProcessor, CalibrationEngine::SensorType::EC); });

    // --- Tuning Handlers ---
    _server.on("/api/v1/tuning/ph/get", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handlePhFilterTuningGet(request, this->_sensorProcessor); });
    _server.on("/api/v1/tuning/ec/get", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleEcFilterTuningGet(request, this->_sensorProcessor); });
    _server.on("/api/v1/tuning/ph/set", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handlePhFilterTuningSet(request, this->_sensorProcessor); });
    _server.on("/api/v1/tuning/ec/set", HTTP_POST, [this](AsyncWebServerRequest *request){ Handlers::handleEcFilterTuningSet(request, this->_sensorProcessor); });

    // --- Sensor Data Handlers ---
    _server.on("/api/v1/calibration/ph/liveVoltage", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handlePhLiveVoltage(request, this->_sensorProcessor); });
    _server.on("/api/v1/calibration/ec/liveVoltage", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleEcLiveVoltage(request, this->_sensorProcessor); });
    _server.on("/api/v1/sensors/raw/phVoltage", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handlePhRawVoltage(request, &g_raw_sensor_data, g_raw_data_mutex); });
    _server.on("/api/v1/sensors/raw/ecVoltage", HTTP_GET, [this](AsyncWebServerRequest *request){ Handlers::handleEcRawVoltage(request, &g_raw_sensor_data, g_raw_data_mutex); });

    // --- WebSocket and Root ---
    _ws.onEvent(onWsEvent);
    _server.addHandler(&_ws);
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){ request->send(200, "text/plain", "Hello from SpHEC Meter"); });
    
    _server.begin();
    LOG_TASK("WebService started.\n");
}

void WebService::sendTelemetry(const std::string& json) {
    _ws.textAll(json.c_str());
}

void WebService::onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch(type) {
        case WS_EVT_CONNECT:
            LOG_MANAGER("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            LOG_MANAGER("WebSocket client #%u disconnected\n", client->id());
            break;
    }
}