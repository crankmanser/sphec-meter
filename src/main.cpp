// src/main.cpp
// MODIFIED FILE

// src/main.cpp
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "DebugMacros.h"
#include "config/hardware_config.h" 
#include "data_models/SensorData_types.h"

// HAL Drivers
#include "hal/INA219_Driver.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "hal/TCA9548_Driver.h" 
#include "hal/PCF8563_Driver.h" 


// Manager Cabinets
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

// Application Layer Cabinets
#include "app/SensorTask.h"
#include "app/ConnectivityTask.h"
#include "app/TelemetryTask.h"
#include "app/TelemetrySerializer.h"
#include "app/WebService.h" 
#include "app/UiTask.h" 
#include "app/StateManager.h"

// Presentation Layer Cabinets
#include "presentation/DisplayManager.h" 
#include "presentation/UIManager.h" 
#include "presentation/screens/HomeScreen.h"

// Debug 
#include "config/DebugConfig.h" 
#if (ENABLE_SENSOR_SIMULATION)
#include "debug/simulation.h"
#endif

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

// --- Manager Pointers ---
// (existing manager pointers...)
TCA9548_Driver* tca9548 = nullptr; 
PCF8563_Driver* pcf8563_driver = nullptr; 
RtcManager* rtcManager = nullptr; 
DisplayManager* displayManager = nullptr; 
UIManager* uiManager = nullptr; 
ButtonManager* buttonManager = nullptr;
EncoderManager* encoderManager = nullptr;
// InputManager* inputManager = nullptr; // To be added
StateManager* stateManager = nullptr;

// (existing manager pointers...)
INA219_Driver* ina219 = nullptr;
ADS1118_Driver* adc1 = nullptr;
ADS1118_Driver* adc2 = nullptr;
DS18B20_Driver* ds18b20 = nullptr;
DHT_Driver* dht = nullptr;
LDR_Driver* ldr = nullptr;
StorageManager* storageManager = nullptr;
PowerManager* powerManager = nullptr;
#if (ENABLE_BLE_STACK)
 BleManager* bleManager = nullptr;
#endif
WifiManager* wifiManager = nullptr;
MqttManager* mqttManager = nullptr;
RawSensorReader* rawSensorReader = nullptr;
LiquidTempManager* liquidTempManager = nullptr;
AmbientTempManager* ambientTempManager = nullptr;
AmbientHumidityManager* ambientHumidityManager = nullptr;
SensorProcessor* sensorProcessor = nullptr;
LDRManager* ldrManager = nullptr;
TelemetrySerializer* telemetrySerializer = nullptr;
WebService* webService = nullptr; 


void setup() {
    LOG_INIT();
    LOG_MAIN("--- SpHEC Meter v1.4.9 Booting ---\n"); 

    g_spi_bus_mutex = xSemaphoreCreateMutex();
    g_raw_data_mutex = xSemaphoreCreateMutex(); 
    g_processed_data_mutex = xSemaphoreCreateMutex(); 
    g_storage_diag_mutex = xSemaphoreCreateMutex();

    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(ADC1_CS_PIN, OUTPUT);
    pinMode(ADC2_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    digitalWrite(ADC1_CS_PIN, HIGH);
    digitalWrite(ADC2_CS_PIN, HIGH);
    delay(10);

    spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    i2c.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // --- Dynamically allocate HAL drivers ---
    ina219 = new INA219_Driver(INA219_I2C_ADDRESS);
    adc1 = new ADS1118_Driver(ADC1_CS_PIN, ADC2_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    adc2 = new ADS1118_Driver(ADC2_CS_PIN, ADC1_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    ds18b20 = new DS18B20_Driver(ONEWIRE_BUS_PIN);
    dht = new DHT_Driver(DHT_PIN, DHT_TYPE);
    ldr = new LDR_Driver(adc1);
    tca9548 = new TCA9548_Driver(TCA_ADDRESS); 
    pcf8563_driver = new PCF8563_Driver();

    // --- Dynamically allocate IO Managers ---
    buttonManager = new ButtonManager(BTN_TOP_PIN, BTN_MIDDLE_PIN, BTN_BOTTOM_PIN);
    encoderManager = new EncoderManager(ENCODER_A_PIN, ENCODER_B_PIN);

    // --- Dynamically allocate Manager & App cabinets ---
    storageManager = new StorageManager(SD_CS_PIN, &spi, g_spi_bus_mutex, g_storage_diag_mutex);
    storageManager->begin();
    storageManager->recoverFromCrash();

    NetworkConfig networkConfig;
    if (!storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig))) {
        LOG_MAIN("No network config file found. Creating default.\n");
        storageManager->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&networkConfig, sizeof(networkConfig));
    }

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
    rtcManager = new RtcManager(*pcf8563_driver, *tca9548);

    // --- Dynamically allocate Presentation Layer & State cabinets --- 
    displayManager = new DisplayManager(*tca9548);
    uiManager = new UIManager(*displayManager);
    stateManager = new StateManager();

    // --- Create and add screens to StateManager ---
    stateManager->addScreen(ScreenState::SCREEN_HOME, new HomeScreen());


    // --- Initialize HAL drivers ---
    ina219->begin(&i2c);
    adc1->begin();
    adc2->begin();
    ds18b20->begin();
    dht->begin();
    tca9548->begin(&i2c); 

    LOG_MAIN("Priming SPI bus...\n");
    adc1->readDifferential_0_1();

    // --- Initialize Managers ---
    powerManager->begin();
    liquidTempManager->begin();
    ambientTempManager->begin();
    ambientHumidityManager->begin();
    sensorProcessor->begin();
    ldrManager->begin();
    telemetrySerializer->begin();
    #if (ENABLE_BLE_STACK)
    bleManager->begin(networkConfig.ble);
    #endif
    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin(); 
    buttonManager->begin();
    encoderManager->begin();
    rtcManager->begin(&i2c);
    displayManager->begin(&i2c); 
    uiManager->begin(); 
    stateManager->begin();

    // --- Create Application Tasks ---
    xTaskCreatePinnedToCore(telemetryTask, "TelemetryTask", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(connectivityTask, "ConnectivityTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, NULL, 3, NULL, 1); // <<< NEW: UI Task on Core 1 with higher priority

    LOG_MAIN("Initialization complete. Entering main loop.\n");
}

void loop() {
    #if (ENABLE_SENSOR_SIMULATION)
        run_test_and_halt();
    #else
        vTaskDelete(NULL);
    #endif
}