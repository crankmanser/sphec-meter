#include "BleManager.h"
#include "DebugMacros.h"

// Constructor for the ServerCallbacks class
BleManager::ServerCallbacks::ServerCallbacks(bool* connectedFlag) {
    _connectedFlag = connectedFlag;
}

// This function is called when the app connects to the device.
void BleManager::ServerCallbacks::onConnect(BLEServer* pServer) {
    *(_connectedFlag) = true;
    LOG_MANAGER("BLE Client Connected\n");
}

// This function is called when the app disconnects.
void BleManager::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    *(_connectedFlag) = false;
    LOG_MANAGER("BLE Client Disconnected\n");
    // It's crucial to start advertising again so the app can reconnect.
    pServer->getAdvertising()->start();
}

// Constructor for the main BleManager class
BleManager::BleManager() :
    _pServer(nullptr),
    _pService(nullptr),
    _pTelemetryCharacteristic(nullptr),
    _clientConnected(false)
{}

void BleManager::begin(const BleConfig& config) {
    LOG_MANAGER("Initializing BleManager...\n");

     // 1. Initialize the core BLE functionality on the ESP32
     BLEDevice::init(config.deviceName);
+
+    // --- FIX: Set a larger MTU ---
+    // The default MTU of 23 bytes is too small for our JSON telemetry, causing instability.
+    BLEDevice::setMTU(517);
 
     // 2. Create the main BLE server object
     _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks(&_clientConnected));

    // 3. Create our custom SpHEC Meter service
   _pService = _pServer->createService(SPHEC_SERVICE_UUID);

    // 4. Create the "telemetry" data channel (characteristic)
    // We set its properties to allow reading and also "notify" (pushing data to the app)
    _pTelemetryCharacteristic = _pService->createCharacteristic(
                                  TELEMETRY_CHARACTERISTIC_UUID,
                                  BLECharacteristic::PROPERTY_READ |
                                  BLECharacteristic::PROPERTY_NOTIFY
                              );
    
    // The BLE2902 descriptor is required by the BLE standard to enable notifications.
    _pTelemetryCharacteristic->addDescriptor(new BLE2902());

    // 5. Start the service to make it active
    _pService->start();

    // 6. Configure and start advertising the device
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SPHEC_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // These settings help with connection stability, especially for iOS devices
    pAdvertising->setMinPreferred(0x0);
    pAdvertising->setMaxPreferred(0x0);
    BLEDevice::startAdvertising();

    LOG_MANAGER("BLE Service started and advertising as '%s'\n", config.deviceName);
}

void BleManager::setTelemetry(const std::string& telemetryJson) {
    // We only send data if the app is actually connected to save power.
    if (_clientConnected) {
        _pTelemetryCharacteristic->setValue(telemetryJson);
        _pTelemetryCharacteristic->notify(); // This pushes the data to the app
    }
}

bool BleManager::isClientConnected() {
    return _clientConnected;
}