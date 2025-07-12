// src/main.cpp
// COMPLETE FILE
#include <Arduino.h>
#include "DebugMacros.h"
#include "boot/init_globals.h"
#include "boot/init_i2c_devices.h"
#include "boot/init_hals.h"
#include "boot/init_managers.h"
#include "boot/post.h"
#include "boot/init_tasks.h"
#include "config/DebugConfig.h"
#if (ENABLE_SENSOR_SIMULATION)
#include "debug/simulation.h"
#endif

#include <SPI.h>
#include <Wire.h>
#include "data_models/SensorData_types.h"
#include "config/Network_Config.h"
#include "hal/INA219_Driver.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "hal/TCA9548_Manual_Driver.h"
#include "hal/PCF8563_Driver.h"
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
#include "managers/rtc/RtcManager.h"
#include "presentation/DisplayManager.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "app/TelemetrySerializer.h"
#include "app/WebService.h"

// --- Global Hardware Handles & Mutexes ---
SPIClass& spi = SPI;
TwoWire i2c = TwoWire(0);
SemaphoreHandle_t g_spi_bus_mutex;
SemaphoreHandle_t g_raw_data_mutex;
SemaphoreHandle_t g_processed_data_mutex;
SemaphoreHandle_t g_storage_diag_mutex;

// --- Global Data Structures ---
RawSensorData g_raw_sensor_data;
ProcessedSensorData g_processed_data;
NetworkConfig networkConfig;

// --- Driver & Manager Pointers ---
// HALs
INA219_Driver* ina219 = nullptr;
ADS1118_Driver* adc1 = nullptr;
ADS1118_Driver* adc2 = nullptr;
DS18B20_Driver* ds18b20 = nullptr;
DHT_Driver* dht = nullptr;
LDR_Driver* ldr = nullptr;
TCA9548_Manual_Driver* tca9548 = nullptr;
PCF8563_Driver* pcf8563_driver = nullptr;
// Managers
StorageManager* storageManager = nullptr;
PowerManager* powerManager = nullptr;
BleManager* bleManager = nullptr;
WifiManager* wifiManager = nullptr;
MqttManager* mqttManager = nullptr;
RawSensorReader* rawSensorReader = nullptr;
LiquidTempManager* liquidTempManager = nullptr;
AmbientTempManager* ambientTempManager = nullptr;
AmbientHumidityManager* ambientHumidityManager = nullptr;
SensorProcessor* sensorProcessor = nullptr;
LDRManager* ldrManager = nullptr;
ButtonManager* buttonManager = nullptr;
EncoderManager* encoderManager = nullptr;
RtcManager* rtcManager = nullptr;
DisplayManager* displayManager = nullptr;
UIManager* uiManager = nullptr;
StateManager* stateManager = nullptr;
TelemetrySerializer* telemetrySerializer = nullptr;
WebService* webService = nullptr;

void setup() {
    LOG_INIT();
    LOG_MAIN("--- SpHEC Meter v1.5.0 Booting ---\n");

    // Instantiate ALL manager and HAL objects first.
    ina219 = new INA219_Driver(INA219_I2C_ADDRESS);
    tca9548 = new TCA9548_Manual_Driver(TCA_ADDRESS, &i2c);
    pcf8563_driver = new PCF8563_Driver();
    displayManager = new DisplayManager(*tca9548);
    rtcManager = new RtcManager(*pcf8563_driver, *tca9548);
    storageManager = new StorageManager(SD_CS_PIN, &spi, g_spi_bus_mutex, g_storage_diag_mutex);
    powerManager = new PowerManager(*ina219, *storageManager);
    #if (ENABLE_BLE_STACK)
    bleManager = new BleManager();
    #endif
    wifiManager = new WifiManager(*storageManager, networkConfig);
    mqttManager = new MqttManager();
    rawSensorReader = new RawSensorReader(&g_raw_sensor_data, adc1, adc2, ina219, ldr, ds18b20, dht);
    liquidTempManager = new LiquidTempManager(&g_raw_sensor_data, &g_processed_data, *storageManager);
    ambientTempManager = new AmbientTempManager(&g_raw_sensor_data, &g_processed_data, *storageManager);
    ambientHumidityManager = new AmbientHumidityManager(&g_raw_sensor_data, &g_processed_data, *storageManager);
    ldrManager = new LDRManager(&g_raw_sensor_data, &g_processed_data, *storageManager);
    sensorProcessor = new SensorProcessor(&g_raw_sensor_data, &g_processed_data, *storageManager);
    telemetrySerializer = new TelemetrySerializer(&g_processed_data, *powerManager);
    webService = new WebService(*storageManager, networkConfig, sensorProcessor, rawSensorReader);
    uiManager = new UIManager(*displayManager);
    stateManager = new StateManager();
    buttonManager = new ButtonManager(BTN_TOP_PIN, BTN_MIDDLE_PIN, BTN_BOTTOM_PIN);
    encoderManager = new EncoderManager(ENCODER_A_PIN, ENCODER_B_PIN);

    init_globals();

    if (!init_i2c_devices()) {
        LOG_MAIN("CRITICAL: I2C Device Initialization Failed. Halting.\n");
        while(true) { delay(1000); }
    }
    
    init_hals();
    init_managers();

    if (run_post()) {
        init_tasks();
        LOG_MAIN("Boot sequence successful. Application starting.\n");
    } else {
        LOG_MAIN("CRITICAL: Power-On Self-Test Failed. Halting.\n");
        while (true) { delay(1000); }
    }
}

void loop() {
    #if (ENABLE_SENSOR_SIMULATION)
        run_test_and_halt();
    #else
        vTaskDelete(NULL);
    #endif
}