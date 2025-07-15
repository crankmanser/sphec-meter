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
#include "app/common/SystemState.h"
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
    // --- STAGE 1: CORE & CONNECTIVITY MANAGERS ---
    // Initialize managers that are required in both NORMAL and DIAGNOSTICS modes.
    // This includes storage, connectivity, and the core UI rendering engine.
    // The StorageManager is now initialized in main.cpp before this function is called.
    storageManager->recoverFromCrash();

    if (!storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig))) {
        LOG_MAIN("No network config file found. Creating default.\n");
        storageManager->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&networkConfig, sizeof(networkConfig));
    }

    #if (ENABLE_BLE_STACK)
    if (bleManager) bleManager->begin(networkConfig.ble);
    #endif
    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin();
    uiManager->begin();
    // Input managers are now initialized in main.cpp before task creation.

    // --- STAGE 2: NORMAL MODE MANAGERS ---
    // Initialize managers that provide the main application logic.
    // These are only needed in NORMAL mode to save resources in DIAGNOSTICS mode.
    if (g_boot_mode == BootMode::NORMAL) {
        powerManager->begin();
        liquidTempManager->begin();
        ambientTempManager->begin();
        ambientHumidityManager->begin();
        ldrManager->begin();
        sensorProcessor->begin();
        telemetrySerializer->begin();
    }

    // --- STAGE 3: STATE & SCREEN INITIALIZATION ---
    // Add all possible screens to the StateManager, then set the initial screen
    // based on the detected boot mode.
    stateManager->addScreen(ScreenState::SCREEN_MAIN_MENU, new MainMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(noiseAnalysisManager));

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