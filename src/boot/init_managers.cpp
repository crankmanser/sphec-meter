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
#include "config/DebugConfig.h"
#include "app/common/SystemState.h" // <<< ADDED
#include "DebugMacros.h"
// Screens
#include "presentation/screens/main_menu/MainMenuScreen.h"
#include "presentation/screens/main_menu/diagnostics/DiagnosticsMenuScreen.h"
#include "presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"

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
extern NoiseAnalysisManager* noiseAnalysisManager;

void init_managers() {
    storageManager->begin();
    storageManager->recoverFromCrash();

    if (!storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig))) {
        LOG_MAIN("No network config file found. Creating default.\n");
        storageManager->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&networkConfig, sizeof(networkConfig));
    }

    // --- Initialize managers that run in ALL modes ---
    #if (ENABLE_BLE_STACK)
    if (bleManager) bleManager->begin(networkConfig.ble);
    #endif
    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin();
    uiManager->begin();
    buttonManager->begin();
    encoderManager->begin();

    // --- Initialize managers that ONLY run in NORMAL mode ---
    if (g_boot_mode == BootMode::NORMAL) {
        powerManager->begin();
        liquidTempManager->begin();
        ambientTempManager->begin();
        ambientHumidityManager->begin();
        ldrManager->begin();
        sensorProcessor->begin();
        telemetrySerializer->begin();
    }
    
    // --- State & Screen Initialization ---
    stateManager->addScreen(ScreenState::SCREEN_MAIN_MENU, new MainMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(noiseAnalysisManager));
    
    // <<< MODIFIED: Set initial screen based on boot mode >>>
    if (g_boot_mode == BootMode::DIAGNOSTICS) {
        LOG_MAIN("Setting initial screen to Diagnostics Menu.\n");
        stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
    } else {
        LOG_MAIN("Setting initial screen to Main Menu.\n");
        stateManager->changeState(ScreenState::SCREEN_MAIN_MENU);
    }
    
    stateManager->begin();

    LOG_MAIN("All managers initialized.\n");
}