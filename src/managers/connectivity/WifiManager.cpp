#include "WifiManager.h"
#include "DebugMacros.h"

WifiManager::WifiManager(StorageManager& storage, NetworkConfig& config) :
    _storage(storage),
    _config(config)
{}

void WifiManager::begin() {
    reconnect();
    LOG_MANAGER("WifiManager initialized.\n");
}

void WifiManager::reconnect() {
    WiFi.disconnect();
    delay(100);
    if (_config.wifi.mode == WifiMode::STATION) {
        if (strlen(_config.wifi.sta_ssid) > 0) {
            LOG_MANAGER("Connecting to %s", _config.wifi.sta_ssid);
            WiFi.begin(_config.wifi.sta_ssid, _config.wifi.sta_password);
            int retries = 0;
            while (WiFi.status() != WL_CONNECTED && retries < 30) {
                delay(500);
                LOG_MANAGER(".");
                retries++;
            }
            if (WiFi.status() == WL_CONNECTED) {
                LOG_MANAGER("\nConnection successful. IP Address: %s\n", WiFi.localIP().toString().c_str());
            } else {
                LOG_MAIN("\n[WM_ERROR] Failed to connect. Falling back to AP mode.\n");
                _config.wifi.mode = WifiMode::ACCESS_POINT;
                if (_storage.saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&_config, sizeof(_config))) {
                   delay(500);
                   ESP.restart();
                }
            }
        } else {
            LOG_MAIN("[WM_ERROR] STA mode selected, but SSID is empty. Switching to AP mode.\n");
            _config.wifi.mode = WifiMode::ACCESS_POINT;
            // No need to save/reboot here, just switch for this session.
            // The AP mode will be activated in the next block.
        }
    }
    
    if (_config.wifi.mode == WifiMode::ACCESS_POINT) {
        if (strlen(_config.wifi.ap_ssid) > 0) {
             WiFi.softAP(_config.wifi.ap_ssid, _config.wifi.ap_password);
        } else {
            WifiConfig defaultConfig;
            LOG_MAIN("[WM_ERROR] AP SSID in config is empty, using default: %s\n", defaultConfig.ap_ssid);
            WiFi.softAP(defaultConfig.ap_ssid, defaultConfig.ap_password);
        }
        LOG_MANAGER("AP Started. IP: %s\n", WiFi.softAPIP().toString().c_str());
    }
}

void WifiManager::update() {
    if (millis() - _lastWatchdogTime > 30000) {
        _lastWatchdogTime = millis();
        if (_config.wifi.mode == WifiMode::STATION) {
            if (WiFi.status() != WL_CONNECTED) {
                LOG_MAIN("[WM] Watchdog: Wi-Fi connection lost. Reconnecting...\n");
                reconnect();
            }
        }
    }
}