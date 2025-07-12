#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <string>
#include "config/Network_Config.h" // <<< NEW: Include the new config file

#define SPHEC_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TELEMETRY_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BleManager {
public:
    BleManager();
    // --- MODIFIED: Accepts the config struct ---
    void begin(const BleConfig& config);
    void setTelemetry(const std::string& telemetryJson);
    bool isClientConnected();

private:
    BLEServer* _pServer;
    BLEService* _pService;
    BLECharacteristic* _pTelemetryCharacteristic;
    bool _clientConnected;

    // A nested class to handle connection and disconnection events.
    // This keeps the callback logic neatly contained within the BleManager.
    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(bool* connectedFlag);
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    private:
        bool* _connectedFlag;
    };
};