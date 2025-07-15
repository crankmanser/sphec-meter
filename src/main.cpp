// src/main.cpp
// MODIFIED FILE
#include <Arduino.h>
#include "DebugMacros.h"
#include "boot/init_globals.h"
#include "boot/init_i2c_devices.h"
#include "boot/init_hals.h"
#include "boot/init_managers.h"
#include "boot/post.h"
#include "boot/init_tasks.h"
#include "config/DebugConfig.h"
#include "app/common/SystemState.h"
#include "presentation/blocks/BootBlock.h"
#include "managers/storage/StorageManager.h"
// ... other includes
#include "app/modes/normal.h"
#include "app/modes/pbios.h"
#include "hal/INA219_Driver.h"
#include "hal/ADS1118_Driver.h"
#include "hal/TCA9548_Manual_Driver.h"
#include "hal/PCF8563_Driver.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
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

// ... (global declarations are unchanged) ...
SPIClass& spi = SPI;
TwoWire i2c = TwoWire(0);
SemaphoreHandle_t g_spi_bus_mutex;
SemaphoreHandle_t g_raw_data_mutex;
SemaphoreHandle_t g_processed_data_mutex;
SemaphoreHandle_t g_storage_diag_mutex;

RawSensorData g_raw_sensor_data;
ProcessedSensorData g_processed_data;
NetworkConfig networkConfig;

TaskHandle_t g_sensorTaskHandle = NULL;
TaskHandle_t g_telemetryTaskHandle = NULL;
TaskHandle_t g_connectivityTaskHandle = NULL;

INA219_Driver* ina219 = nullptr;
ADS1118_Driver* adc1 = nullptr;
ADS1118_Driver* adc2 = nullptr;
DS18B20_Driver* ds18b20 = nullptr;
DHT_Driver* dht = nullptr;
LDR_Driver* ldr = nullptr;
TCA9548_Manual_Driver* tca9548 = nullptr;
PCF8563_Driver* pcf8563_driver = nullptr;
StorageManager* storageManager = nullptr;
DisplayManager* displayManager = nullptr;
ButtonManager* buttonManager = nullptr;
EncoderManager* encoderManager = nullptr;
StateManager* stateManager = nullptr;
UIManager* uiManager = nullptr;
RtcManager* rtcManager = nullptr;
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
TelemetrySerializer* telemetrySerializer = nullptr;
WebService* webService = nullptr;
NoiseAnalysisManager* noiseAnalysisManager = nullptr;

BootMode g_boot_mode = BootMode::NORMAL;


void setup() {
    // --- BOOTLOADER SEQUENCE ---
    LOG_INIT();
    LOG_MAIN("--- SpHEC Meter v1.6.9 Bootloader ---\n");
    init_globals();

    // ... (Object instantiation is unchanged) ...
    ina219 = new INA219_Driver(INA219_I2C_ADDRESS);
    tca9548 = new TCA9548_Manual_Driver(TCA_ADDRESS, &i2c);
    pcf8563_driver = new PCF8563_Driver();
    adc1 = new ADS1118_Driver(ADC1_CS_PIN, ADC2_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    adc2 = new ADS1118_Driver(ADC2_CS_PIN, ADC1_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    ds18b20 = new DS18B20_Driver(ONEWIRE_BUS_PIN);
    dht = new DHT_Driver(DHT_PIN, DHT_TYPE);
    ldr = new LDR_Driver(adc1);
    displayManager = new DisplayManager(*tca9548, &i2c);
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
    encoderManager = new EncoderManager();


    // --- Initialize hardware required for boot decisions ---
    storageManager->begin();
    if (!init_i2c_devices()) {
        LOG_MAIN("CRITICAL: I2C Device Initialization Failed. Halting.\n");
        while(true) { delay(1000); }
    }

    // --- Perform recovery if necessary ---
    if (!storageManager->checkAndClearShutdownFlag()) {
        BootBlockProps props;
        props.title = "Recovery";
        props.message = "Checking files...";
        BootBlock::draw(displayManager, props);
        storageManager->requestRecovery(); // <<< MODIFIED: Call the non-blocking method
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for the async operation to likely complete
    }
    
    // --- Detect user intent and hand off control ---
    bool pBiosRequested = (digitalRead(BTN_MIDDLE_PIN) == LOW && digitalRead(BTN_BOTTOM_PIN) == LOW);
    g_boot_mode = pBiosRequested ? BootMode::DIAGNOSTICS : BootMode::NORMAL;
    
    // <<< MODIFIED: Instantiate NoiseAnalysisManager only if needed >>>
    if(g_boot_mode == BootMode::DIAGNOSTICS) {
        noiseAnalysisManager = new NoiseAnalysisManager(adc1, adc2, ina219, nullptr, nullptr, nullptr);
        run_pbios_mode();
    } else {
        run_normal_mode();
    }
}

void loop() {
    vTaskDelete(NULL);
}