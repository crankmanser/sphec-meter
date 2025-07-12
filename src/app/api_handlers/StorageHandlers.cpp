// src/app/api_handlers/StorageHandlers.cpp
#include "StorageHandlers.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>
#include "helpers/Base64.h"

namespace Handlers {

void handleStorageDiagnosticsStart(AsyncWebServerRequest *request, StorageManager* storage) {
    if (storage->requestDiagnostics()) {
        request->send(202, "application/json", "{\"status\":\"accepted\", \"message\":\"Storage diagnostic test initiated.\"}");
    } else {
        request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Failed to queue diagnostic test.\"}");
    }
}

void handleStorageDiagnosticsResult(AsyncWebServerRequest *request, StorageManager* storage) {
    StorageDiagnosticResult result = storage->getDiagnosticResult();

    DynamicJsonDocument doc(256);
    doc["status"] = "ok";
    doc["timestamp"] = result.timestamp;
    doc["passed"] = result.is_passed;

    JsonObject details = doc.createNestedObject("details");
    details["sd_card_initialized"] = result.is_sd_card_initialized;
    details["can_create_file"] = result.can_create_file;
    details["can_write_file"] = result.can_write_file;
    details["can_read_file"] = result.can_read_file;
    details["can_delete_file"] = result.can_delete_file;
    details["free_space_kb"] = result.free_space_kb;

    std::string response;
    serializeJson(doc, response);
    request->send(200, "application/json", response.c_str());
}

void handleBackup(AsyncWebServerRequest *request, StorageManager* storage) {
    LOG_MANAGER("Handling backup request...\n");
    DynamicJsonDocument doc(2048); 

    std::vector<uint8_t> networkConfData = storage->readFile(ConfigType::NETWORK_CONFIG);
    if (!networkConfData.empty()) {
        doc[String(F("/network.conf"))] = base64_encode(networkConfData.data(), networkConfData.size());
    }

    std::vector<uint8_t> powerStateData = storage->readFile(ConfigType::POWER_MANAGER_STATE);
    if (!powerStateData.empty()) {
        doc[String(F("/power.state"))] = base64_encode(powerStateData.data(), powerStateData.size());
    }
    
    std::vector<uint8_t> phCalData = storage->readFile(ConfigType::PH_CALIBRATION);
    if (!phCalData.empty()) {
        doc[String(F("/ph_cal.json"))] = base64_encode(phCalData.data(), phCalData.size());
    }
    std::vector<uint8_t> ecCalData = storage->readFile(ConfigType::EC_CALIBRATION);
    if (!ecCalData.empty()) {
        doc[String(F("/ec_cal.json"))] = base64_encode(ecCalData.data(), ecCalData.size());
    }

    std::vector<uint8_t> filterTuningData = storage->readFile(ConfigType::FILTER_TUNING_CONFIG);
    if (!filterTuningData.empty()) {
        doc[String(F("/filter_tuning.json"))] = base64_encode(filterTuningData.data(), filterTuningData.size());
    }

    std::string responseJson;
    serializeJson(doc, responseJson);

    request->send(200, "application/json", responseJson.c_str());
    LOG_MANAGER("Backup response sent.\n");
}

void handleSetConfig(AsyncWebServerRequest *request, StorageManager* storage, NetworkConfig* config) {
    LOG_MANAGER("Received network config update via lean API\n");

    int params = request->params();
    bool configChanged = false;
    for(int i=0; i<params; i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            configChanged = true;
            if (p->name() == "wifi_mode") {
                if (p->value().equalsIgnoreCase("ap")) config->wifi.mode = WifiMode::ACCESS_POINT;
                else config->wifi.mode = WifiMode::STATION;
            }
            if (p->name() == "sta_ssid") strlcpy(config->wifi.sta_ssid, p->value().c_str(), sizeof(config->wifi.sta_ssid));
            if (p->name() == "sta_password") strlcpy(config->wifi.sta_password, p->value().c_str(), sizeof(config->wifi.sta_password));
            if (p->name() == "mqtt_host") strlcpy(config->mqtt.host, p->value().c_str(), sizeof(config->mqtt.host));
            if (p->name() == "mqtt_port") config->mqtt.port = p->value().toInt();
            if (p->name() == "mqtt_user") strlcpy(config->mqtt.username, p->value().c_str(), sizeof(config->mqtt.username));
            if (p->name() == "mqtt_pass") strlcpy(config->mqtt.password, p->value().c_str(), sizeof(config->mqtt.password));
        }
    }

    if (configChanged) {
        if (storage->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)config, sizeof(NetworkConfig))) {
            LOG_MANAGER("New network config saved. Rebooting.\n");
            request->send(200, "application/json", "{\"status\":\"ok\", \"rebooting\":true}");
            delay(1000);
            ESP.restart();
        } else {
            request->send(500, "application/json", "{\"status\":\"error\", \"message\":\"Failed to save config\"}");
        }
    } else {
        request->send(400, "application/json", "{\"status\":\"error\", \"message\":\"No parameters received\"}");
    }
}

void handleRestoreUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, StorageManager* storage) {
    if(!index){
        LOG_MANAGER("Restore upload started: %s\n", filename.c_str());
    }
    if(final){
        LOG_MANAGER("Restore upload finished. Total size: %u bytes.\n", index + len);
        std::vector<uint8_t> backupData(data, data + len);
        storage->restoreBackup(backupData);
    }
}

} // namespace Handlers