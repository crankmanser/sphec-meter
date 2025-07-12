#pragma once

// =================================================================
// Wi-Fi Configuration
// =================================================================

// Defines the operational mode for the Wi-Fi manager.
enum class WifiMode {
    ACCESS_POINT,
    STATION
};

// Defines the structure for holding all Wi-Fi settings.
struct WifiConfig {
    // General Setting
    WifiMode mode = WifiMode::STATION;

    // Settings for when the device is a Station (connecting to a network)
    char sta_ssid[33] = "Fortuna"; 
    char sta_password[65] = "H4rrOW42";

    // Settings for when the device is an Access Point (creating its own network)
    char ap_ssid[33] = "SpHEC-Meter"; 
    char ap_password[65] = "password";
};


// =================================================================
// Bluetooth Low Energy (BLE) Configuration
// =================================================================

// Defines the structure for holding all BLE settings.
struct BleConfig {
    // The name the device will advertise over BLE.
    char deviceName[33] = "SpHEC Meter";
};


// =================================================================
// MQTT Configuration
// =================================================================
struct MqttConfig {
    bool enabled = true;
    char host[65] = "192.168.3.140";
    uint16_t port = 1883;
    char username[33] = "";
    char password[33] = "";
    char command_topic[128] = "sphec/meter/command";
    char telemetry_topic[128] = "sphec/meter/telemetry";
};


// =================================================================
// Master Network Configuration
// =================================================================

// This is the master structure that encapsulates all network settings.
// An instance of THIS struct will be saved to a single file on the SD card.
struct NetworkConfig {
    WifiConfig wifi;
    BleConfig ble;
    MqttConfig mqtt;
};