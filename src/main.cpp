// src/main.cpp
// MODIFIED FILE
#include <Arduino.h>
#include "DebugMacros.h"
#include "app/globals.h"
#include "app/AppContext.h"

// --- Include all necessary headers ---
#include "config/hardware_config.h"
#include "hal/TCA9548_Manual_Driver.h"
#include "hal/INA219_Driver.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "hal/PCF8563_Driver.h"
#include "managers/storage/StorageManager.h"
#include "presentation/DisplayManager.h"
#include "managers/rtc/RtcManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "managers/power/PowerManager.h"
#include "managers/sensor/RawSensorReader.h"
#include "managers/sensor/LiquidTempManager.h"
#include "managers/sensor/AmbientTempManager.h"
#include "managers/sensor/AmbientHumidityManager.h"
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/LDRManager.h"
#include "app/TelemetrySerializer.h"
#include "managers/connectivity/WifiManager.h"
#include "managers/connectivity/MqttManager.h"
#include "app/WebService.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#if (ENABLE_BLE_STACK)
#include "managers/connectivity/BleManager.h"
#endif

// --- Task Headers ---
#include "app/UiTask.h"
#include "app/ConnectivityTask.h"
#include "app/SensorTask.h"
#include "app/TelemetryTask.h"
#include "app/EncoderTask.h"

// --- Boot Headers ---
#include "boot/init_i2c_devices.h"
#include "boot/init_hals.h"
#include "boot/init_managers.h"
#include "boot/init_tasks.h"
#include "boot/post.h"
#include "app/modes/normal.h"
#include "app/modes/pbios.h"

// --- Global Objects (Definitions) ---
SPIClass spi(VSPI);
TwoWire i2c(0);
SemaphoreHandle_t g_spi_bus_mutex;
SemaphoreHandle_t g_raw_data_mutex;
SemaphoreHandle_t g_processed_data_mutex;
SemaphoreHandle_t g_storage_diag_mutex;

BootMode g_boot_mode = BootMode::NORMAL;
NetworkConfig networkConfig;
RawSensorData g_raw_sensor_data;
ProcessedSensorData g_processed_data;
AppContext appContext;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    LOG_INIT();
    LOG_MAIN("\n\n--- SpHEC Meter v1.7.2 Final ---\n");

    // --- PHASE 1: Create RTOS Primitives ---
    g_spi_bus_mutex = xSemaphoreCreateMutex();
    g_raw_data_mutex = xSemaphoreCreateMutex();
    g_processed_data_mutex = xSemaphoreCreateMutex();
    g_storage_diag_mutex = xSemaphoreCreateMutex();
    
    // --- PHASE 2: Initialize Hardware Buses ---
    i2c.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

    // --- PHASE 3: Instantiate all objects ---
    appContext.tca = new TCA9548_Manual_Driver(TCA_ADDRESS, &i2c);
    appContext.ina219 = new INA219_Driver(INA219_I2C_ADDRESS);
    appContext.adc1 = new ADS1118_Driver(ADC1_CS_PIN, ADC2_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    appContext.adc2 = new ADS1118_Driver(ADC2_CS_PIN, ADC1_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    appContext.ds18b20 = new DS18B20_Driver(ONEWIRE_BUS_PIN);
    appContext.dht = new DHT_Driver(DHT_PIN, DHT_TYPE);
    appContext.ldr = new LDR_Driver(appContext.adc1);
    appContext.rtc_driver = new PCF8563_Driver();
    appContext.displayManager = new DisplayManager(*appContext.tca, &i2c);
    appContext.storageManager = new StorageManager(SD_CS_PIN, &spi, g_spi_bus_mutex, g_storage_diag_mutex);
    appContext.rtcManager = new RtcManager(*appContext.rtc_driver, *appContext.tca);
    appContext.buttonManager = new ButtonManager(BTN_TOP_PIN, BTN_MIDDLE_PIN, BTN_BOTTOM_PIN);
    appContext.encoderManager = new EncoderManager();
    appContext.stateManager = new StateManager();
    appContext.uiManager = new UIManager(*appContext.displayManager);
    appContext.powerManager = new PowerManager(*appContext.ina219, *appContext.storageManager);
    appContext.rawSensorReader = new RawSensorReader(&g_raw_sensor_data, appContext.adc1, appContext.adc2, appContext.ina219, appContext.ldr, appContext.ds18b20, appContext.dht);
    appContext.liquidTempManager = new LiquidTempManager(&g_raw_sensor_data, &g_processed_data, *appContext.storageManager);
    appContext.ambientTempManager = new AmbientTempManager(&g_raw_sensor_data, &g_processed_data, *appContext.storageManager);
    appContext.ambientHumidityManager = new AmbientHumidityManager(&g_raw_sensor_data, &g_processed_data, *appContext.storageManager);
    appContext.sensorProcessor = new SensorProcessor(&g_raw_sensor_data, &g_processed_data, *appContext.storageManager);
    appContext.ldrManager = new LDRManager(&g_raw_sensor_data, &g_processed_data, *appContext.storageManager);
    appContext.telemetrySerializer = new TelemetrySerializer(&g_processed_data, *appContext.powerManager);
    appContext.wifiManager = new WifiManager(*appContext.storageManager, networkConfig);
    appContext.mqttManager = new MqttManager();
    appContext.webService = new WebService(*appContext.storageManager, networkConfig, appContext.sensorProcessor, appContext.rawSensorReader);
    appContext.noiseAnalysisManager = new NoiseAnalysisManager(appContext.adc1, appContext.adc2, appContext.ina219, nullptr, nullptr, nullptr);
    #if (ENABLE_BLE_STACK)
    appContext.bleManager = new BleManager();
    #endif

    // --- PHASE 4: Sequential Hardware Initialization ---
    init_i2c_devices(&appContext);
    init_hals(&appContext);
    appContext.storageManager->begin();

    // --- PHASE 5: Detect Boot Mode ---
    appContext.buttonManager->begin();
    appContext.encoderManager->begin();
    pinMode(BTN_MIDDLE_PIN, INPUT_PULLUP);
    pinMode(BTN_BOTTOM_PIN, INPUT_PULLUP);
    delay(10); 
    bool pBiosRequested = (digitalRead(BTN_MIDDLE_PIN) == LOW && digitalRead(BTN_BOTTOM_PIN) == LOW);
    g_boot_mode = pBiosRequested ? BootMode::DIAGNOSTICS : BootMode::NORMAL;
    LOG_BOOT("Boot Mode: %s\n", pBiosRequested ? "DIAGNOSTICS" : "NORMAL");

    // --- PHASE 6: Initialize Managers ---
    init_managers(&appContext);
    
    // --- PHASE 7: Start RTOS Tasks ---
    LOG_BOOT("Creating RTOS Tasks...\n");
    init_tasks(&appContext);

    LOG_MAIN("Setup complete. Deleting loopTask.\n");
    vTaskDelete(NULL);
}

void loop() {}