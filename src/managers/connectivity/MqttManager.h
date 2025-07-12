#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <AsyncMqttClient.h>
#include "config/Network_Config.h"
#include <string>

class MqttManager {
public:
    MqttManager();
    void begin(const MqttConfig& config);
    void update(); // For maintaining connection
    void publishTelemetry(const std::string& payload);

private:
    AsyncMqttClient _mqttClient;
    MqttConfig _config;
    unsigned long _lastReconnectAttempt = 0;

    void connectToMqtt();
    
    // Static callbacks required by the library
    static void onMqttConnect(bool sessionPresent);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
};

#endif // MQTTMANAGER_H