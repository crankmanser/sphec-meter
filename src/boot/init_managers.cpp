// src/boot/init_managers.cpp
// MODIFIED FILE
#include "init_managers.h"
#include "app/AppContext.h" // <<< ADDED: Include full context definition
#include "DebugMacros.h"

// Include all necessary managers and screens
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
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "app/TelemetrySerializer.h"
#include "app/WebService.h"
#include "config/DebugConfig.h"
#include "app/common/SystemState.h"
#include "presentation/screens/main_menu/MainMenuScreen.h"
#include "presentation/screens/main_menu/diagnostics/DiagnosticsMenuScreen.h"
#include "presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include "config/Network_Config.h" // Required for networkConfig extern

// External global, will be phased out
extern NetworkConfig networkConfig;

void init_managers(AppContext* appContext) {
    // Start the RTOS components of the StorageManager.
    appContext->storageManager->startRtosDependencies();

    // Load the network config from storage into the global struct.
    appContext->storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig));
    
    // Initialize the rest of the managers using the context
    #if (ENABLE_BLE_STACK)
    if (appContext->bleManager) appContext->bleManager->begin(networkConfig.ble);
    #endif
    appContext->wifiManager->begin();
    appContext->mqttManager->begin(networkConfig.mqtt);
    appContext->webService->begin();
    appContext->uiManager->begin(); 

    // Initialize managers that only run in NORMAL mode
    if (g_boot_mode == BootMode::NORMAL) {
        appContext->powerManager->begin();
        appContext->liquidTempManager->begin();
        appContext->ambientTempManager->begin();
        appContext->ambientHumidityManager->begin();
        appContext->ldrManager->begin();
        appContext->sensorProcessor->begin();
        appContext->telemetrySerializer->begin();
    }
    
    // --- STATE MANAGER & SCREEN SETUP ---
    appContext->stateManager->addScreen(ScreenState::SCREEN_MAIN_MENU, new MainMenuScreen());
    
    if (appContext->noiseAnalysisManager != nullptr) {
        appContext->stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
        appContext->stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(appContext->noiseAnalysisManager));
    }
    
    if (g_boot_mode == BootMode::DIAGNOSTICS) {
        appContext->stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
    } else {
        appContext->stateManager->changeState(ScreenState::SCREEN_MAIN_MENU);
    }
    
    // <<< MODIFIED: Pass the context to the StateManager's begin method >>>
    appContext->stateManager->begin(appContext);

    LOG_MANAGER("Manager initialization complete.\n");
}