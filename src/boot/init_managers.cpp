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
    // --- STAGE 1: UNIVERSAL MANAGERS ---
    // The StorageManager's recovery is now handled in main.cpp before this.
    // We just need to load the network configuration.
    if (!storageManager->loadState(ConfigType::NETWORK_CONFIG, (uint8_t*)&networkConfig, sizeof(networkConfig))) {
        storageManager->saveState(ConfigType::NETWORK_CONFIG, (const uint8_t*)&networkConfig, sizeof(networkConfig));
    }
    #if (ENABLE_BLE_STACK)
    if (bleManager) bleManager->begin(networkConfig.ble);
    #endif
    wifiManager->begin();
    mqttManager->begin(networkConfig.mqtt);
    webService->begin();
    uiManager->begin(); 

    // --- STAGE 2: NORMAL MODE MANAGERS ---
    if (g_boot_mode == BootMode::NORMAL) {
        powerManager->begin();
        liquidTempManager->begin();
        ambientTempManager->begin();
        ambientHumidityManager->begin();
        ldrManager->begin();
        sensorProcessor->begin();
        telemetrySerializer->begin();
    }
    
    // --- STAGE 3: STATE MANAGER & SCREEN SETUP ---
    stateManager->addScreen(ScreenState::SCREEN_MAIN_MENU, new MainMenuScreen());
    
    // Conditionally add the Diagnostics screens only if the manager exists (i.e., in pBios mode)
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