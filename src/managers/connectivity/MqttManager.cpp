#include "MqttManager.h"
#include "DebugMacros.h"
#include <WiFi.h>

// A global pointer to the MqttManager instance is needed for the C-style static callbacks
static MqttManager* _mqttManagerInstance = nullptr;

MqttManager::MqttManager() {
    _mqttManagerInstance = this;
}

void MqttManager::begin(const MqttConfig& config) {
    _config = config;

    if (!_config.enabled) {
        LOG_MANAGER("MQTT is disabled in config.\n");
        return;
    }

    LOG_MANAGER("Initializing MQTT Manager...\n");

    _mqttClient.onConnect(onMqttConnect);
    _mqttClient.onDisconnect(onMqttDisconnect);
    _mqttClient.onMessage(onMqttMessage);
    
    _mqttClient.setServer(_config.host, _config.port);
    if (strlen(_config.username) > 0) {
        _mqttClient.setCredentials(_config.username, _config.password);
    }
}

void MqttManager::update() {
    if (!_config.enabled || !WiFi.isConnected() || _mqttClient.connected()) {
        return;
    }

    // Non-blocking reconnect timer
    if (millis() - _lastReconnectAttempt > 5000) {
        _lastReconnectAttempt = millis();
        LOG_MANAGER("Attempting MQTT connection...\n");
        connectToMqtt();
    }
}

void MqttManager::connectToMqtt() {
    if (WiFi.isConnected()) {
        _mqttClient.connect();
    }
}

void MqttManager::publishTelemetry(const std::string& payload) {
    if (!_config.enabled || !_mqttClient.connected()) {
        return;
    }
    // Publish with QoS 1, not retained
    _mqttClient.publish(_config.telemetry_topic, 1, false, payload.c_str(), payload.length());
}

void MqttManager::onMqttConnect(bool sessionPresent) {
    LOG_MANAGER("Connected to MQTT broker.\n");
    // Subscribe to the command topic with QoS 1
    _mqttManagerInstance->_mqttClient.subscribe(_mqttManagerInstance->_config.command_topic, 1);
}

void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    LOG_MAIN("[MQTT_ERROR] Disconnected from MQTT. Reason: %d\n", static_cast<int>(reason));
}

void MqttManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    LOG_MANAGER("MQTT message received.\n");
    LOG_MANAGER("  Topic: %s\n", topic);
    
    // In the future, this is where we'll parse JSON commands
    std::string message;
    message.assign(payload, len);
    LOG_MANAGER("  Payload: %s\n", message.c_str());
}