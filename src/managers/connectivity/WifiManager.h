#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include "managers/storage/StorageManager.h" 
#include "config/Network_Config.h"

/**
 * @class WifiManager
 * @brief A Manager layer cabinet responsible only for managing the Wi-Fi connection state.
 */
class WifiManager {
public:
    WifiManager(StorageManager& storage, NetworkConfig& config);
    void begin();
    void update();

private:
    StorageManager& _storage;
    NetworkConfig& _config;
    unsigned long _lastWatchdogTime = 0;

    void reconnect();
};

#endif // WIFIMANAGER_H