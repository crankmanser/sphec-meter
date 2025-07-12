// src/boot/init_managers.cpp
// COMPLETE AND CORRECTED FILE
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
#include "managers/rtc/RtcManager.h"
#include "presentation/DisplayManager.h"
#include "hal/PCF8563_Driver.h"
#include "hal/TCA9548_Manual_Driver.h"

// External declarations for manager pointers and HAL driver pointers
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
extern RtcManager* rtcManager;
extern DisplayManager* displayManager;
extern UIManager* uiManager;
extern StateManager* stateManager;
extern TelemetrySerializer* telemetrySerializer;
extern WebService* webService;

extern INA219_Driver* ina219;
extern ADS1118_Driver* adc1;
extern ADS1118_Driver* adc2;
extern DS18B20_Driver* ds18b20;
extern DHT_Driver* dht;
extern LDR_Driver* ldr;
extern TCA9548_Manual_Driver* tca9548;
extern PCF8563_Driver* pcf8563_driver;

extern SPIClass& spi;
extern TwoWire i2c;
extern SemaphoreHandle_t g_spi_bus_mutex;
extern SemaphoreHandle_t g_storage_diag_mutex;
extern RawSensorData g_raw_sensor_data;
extern ProcessedSensorData g_processed_data;
extern NetworkConfig networkConfig;

void init_managers() {
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