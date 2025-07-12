// src/boot/init_managers.cpp
// MODIFIED FILE
#include "init_managers.h"
#include "managers/storage/StorageManager.h"
#include "managers/power/PowerManager.h"
#include "managers/connectivity/BleManager.h"
#include "managers/connectivity/WifiManager.h"
#include "managers/connectivity/MqttManager.h"
#include "managers/sensor/RawSensorReader.h"
#include "managers/sensor/LiquidTempManager.h"
#include "managers/sensor/AmbientTempManager.h"
#include "managers/sensor/AmbientHumidityManager.h"
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/LDRManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "app/TelemetrySerializer.h"
#include "app/WebService.h"
#include "presentation/screens/HomeScreen.h"
#include "config/DebugConfig.h"
#include "DebugMacros.h"
// NOTE: RtcManager header is no longer needed here.

// External declarations for manager pointers
extern StorageManager* storageManager;
extern PowerManager* powerManager;
extern BleManager* bleManager;
extern WifiManager* wifiManager;
extern MqttManager* mqttManager;
extern RawSensorReader* rawSensorReader;
extern LiquidTempManager* liquidTempManager;
extern AmbientTempManager* ambientTempManager;
extern AmbientHumidityManager* ambientHumidityManager;
extern SensorProcessor* sensorProcessor;
extern LDRManager* ldrManager;
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;
extern UIManager* uiManager;
extern StateManager* stateManager;
extern TelemetrySerializer* telemetrySerializer;
extern WebService* webService;
extern NetworkConfig networkConfig;

void init_managers() {
    // MODIFICATION: RtcManager->begin() is REMOVED from this function.
    // It is now correctly handled in init_i2c_devices().

    // Manager Initialization (Non-I2C managers that are safe to init now)
    storageManager->begin();
    storageManager->recoverFromCrash();

    if (!storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig))) {
        LOG_MAIN("No network config file found. Creating default.\n");
        storageManager->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&networkConfig, sizeof(networkConfig));
    }

    powerManager->begin();
    liquidTempManager->begin();
    ambientTempManager->begin();
    ambientHumidityManager->begin();
    ldrManager->begin();
    sensorProcessor->begin();
    telemetrySerializer->begin();

    #if (ENABLE_BLE_STACK)
    if (bleManager) bleManager->begin(networkConfig.ble);
    #endif

    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin();
    uiManager->begin();
    stateManager->addScreen(ScreenState::SCREEN_HOME, new HomeScreen());
    stateManager->begin();
    buttonManager->begin();
    encoderManager->begin();

    LOG_MAIN("All managers initialized.\n");
}