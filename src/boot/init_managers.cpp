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

void init_managers(NoiseAnalysisManager* noiseAnalysisManager) {
    // Start the RTOS components of the StorageManager (task and queue).
    storageManager->startRtosDependencies();

    // --- FIX: Removed the dangerous load/save block ---
    // The main boot thread no longer tries to save default files.
    // We only attempt to load the config. If it fails, the global networkConfig
    // struct already holds safe, in-memory defaults for the current session.
    // The StorageTask will create the default file in the background for the next boot.
    storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig));
    
    // Initialize the rest of the managers
    #if (ENABLE_BLE_STACK)
    if (bleManager) bleManager->begin(networkConfig.ble);
    #endif
    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin();
    uiManager->begin(); 

    // Initialize managers that only run in NORMAL mode
    if (g_boot_mode == BootMode::NORMAL) {
        powerManager->begin();
        liquidTempManager->begin();
        ambientTempManager->begin();
        ambientHumidityManager->begin();
        ldrManager->begin();
        sensorProcessor->begin();
        telemetrySerializer->begin();
    }
    
    // --- STATE MANAGER & SCREEN SETUP ---
    stateManager->addScreen(ScreenState::SCREEN_MAIN_MENU, new MainMenuScreen());
    
    if (noiseAnalysisManager != nullptr) {
        stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
        stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(noiseAnalysisManager));
    }
    
    if (g_boot_mode == BootMode::DIAGNOSTICS) {
        stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
    } else {
        stateManager->changeState(ScreenState::SCREEN_MAIN_MENU);
    }
    stateManager->begin();

    LOG_MANAGER("Manager initialization complete.\n");
}
