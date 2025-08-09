// File Path: /src/main.cpp
// MODIFIED FILE

#ifndef PIO_UNIT_TESTING
#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
#include <vector>
#include <numeric>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <AdcManager.h>
#include <SdManager.h>
#include <TempManager.h>
#include <FilterManager.h>
#include <CalibrationManager.h>
#include "ui/InputManager.h"
#include "ui/StateManager.h"
#include "ui/UIManager.h"
#include "boot/boot_sequence.h"
#include "pBiosContext.h"
#include "GuidedTuningEngine.h"
#include "HardwareTester.h"

// --- DEFINITIVE UPDATE: Include all new screen headers ---
#include "ui/screens/ProbeProfilingScreen.h" 
#include "ui/screens/MaintenanceScreen.h"
#include "ui/screens/ShutdownScreen.h"
#include "ui/screens/AutoTuningScreen.h"
#include "ui/screens/pBiosMenuScreen.h"
#include "ui/screens/FilterSelectionScreen.h"
#include "ui/screens/LiveFilterTuningScreen.h"
#include "ui/screens/ParameterEditScreen.h"
#include "ui/screens/LiveVoltmeterScreen.h"
#include "ui/screens/HardwareTestScreen.h"
#include "ui/screens/NoiseAnalysisScreen.h"
#include "ui/screens/DriftTrendingScreen.h"
#include "ui/screens/AutoTuneSubMenuScreen.h"
#include "ui/screens/MainMenuScreen.h"

// Global objects
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;
TempManager tempManager;
FilterManager phFilter, ecFilter, v3_3_Filter, v5_0_Filter;
CalibrationManager phCalManager, ecCalManager;
InputManager inputManager;
StateManager* stateManager = nullptr; 
UIManager* uiManager = nullptr;
GuidedTuningEngine guidedTuningEngine;
HardwareTester hardwareTester;

PBiosContext pBiosContext;
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;

// Task forward declarations
void uiTask(void* pvParameters);
void dataTask(void* pvParameters);
void oneWireTask(void* pvParameters);

void setup() {
    Serial.begin(115200);
    delay(100);

    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    delay(50); 
    BootMode selected_mode = (digitalRead(BTN_ENTER_PIN) == LOW) ? BootMode::PBIOS : BootMode::NORMAL;

    faultHandler.begin();
    displayManager.begin(faultHandler);
    
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    
    spiMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    
    adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    configManager.begin(faultHandler, sdManager);
    tempManager.begin(faultHandler);

    phFilter.begin(faultHandler, configManager, "ph_filter");
    ecFilter.begin(faultHandler, configManager, "ec_filter");
    v3_3_Filter.begin(faultHandler, configManager, "v3_3_filter");
    v5_0_Filter.begin(faultHandler, configManager, "v5_0_filter");
    phCalManager.begin(faultHandler);
    ecCalManager.begin(faultHandler);
    StaticJsonDocument<512> phCalDoc, ecCalDoc;
    if (sdManager.loadJson("/ph_cal.json", phCalDoc)) { 
        phCalManager.deserializeModel(phCalManager.getMutableCurrentModel(), phCalDoc);
    }
    if (sdManager.loadJson("/ec_cal.json", ecCalDoc)) { 
        ecCalManager.deserializeModel(ecCalManager.getMutableCurrentModel(), ecCalDoc);
    }

    inputManager.begin(); 
    if (selected_mode == BootMode::PBIOS) {
        while(digitalRead(BTN_ENTER_PIN) == LOW) { delay(10); }
        inputManager.clearEnterButtonState();
    }

    xTaskCreatePinnedToCore(uiTask, "uiTask", 10240, (void*)selected_mode, 3, NULL, 1);
    xTaskCreatePinnedToCore(dataTask, "dataTask", 10240, (void*)selected_mode, 2, NULL, 0);
    xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, 1, NULL);
    
    adcManager.setProbeState(0, ProbeState::ACTIVE);
    adcManager.setProbeState(1, ProbeState::ACTIVE);
    LOG_BOOT("Initialization Complete. Mode: %s", selected_mode == BootMode::PBIOS ? "pBIOS" : "Normal");
}

void loop() { 
    vTaskSuspend(NULL); 
}

void uiTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("UI Task started on Core %d", xPortGetCoreID());
    
    uiManager = new UIManager(displayManager);
    stateManager = new StateManager();
    stateManager->begin();
    
    if (mode == BootMode::PBIOS) {
        stateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
        stateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
        // --- DEFINITIVE FIX: Register the same UI screen for all wizard states ---
        AutoTuningScreen* tuneScreen = new AutoTuningScreen();
        stateManager->addScreen(ScreenState::AUTO_TUNE_CHARACTERIZE_SIGNAL, tuneScreen);
        stateManager->addScreen(ScreenState::AUTO_TUNE_OPTIMIZE_HF, tuneScreen);
        stateManager->addScreen(ScreenState::AUTO_TUNE_OPTIMIZE_LF, tuneScreen);
        stateManager->addScreen(ScreenState::AUTO_TUNE_FINALIZE, tuneScreen);

        stateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, new LiveFilterTuningScreen(&adcManager, &pBiosContext, &phCalManager, &ecCalManager, &tempManager));
        stateManager->addScreen(ScreenState::PARAMETER_EDIT, new ParameterEditScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::MAINTENANCE_MENU, new MaintenanceScreen());
        stateManager->addScreen(ScreenState::SHUTDOWN_MENU, new ShutdownScreen());
        stateManager->addScreen(ScreenState::LIVE_VOLTMETER, new LiveVoltmeterScreen());
        stateManager->addScreen(ScreenState::HARDWARE_SELF_TEST, new HardwareTestScreen());
        stateManager->addScreen(ScreenState::PROBE_PROFILING, new ProbeProfilingScreen());
        stateManager->addScreen(ScreenState::NOISE_ANALYSIS, new NoiseAnalysisScreen(&pBiosContext, &adcManager));
        stateManager->addScreen(ScreenState::DRIFT_TRENDING, new DriftTrendingScreen(&pBiosContext, &adcManager));
        stateManager->addScreen(ScreenState::AUTO_TUNE_SUB_MENU, new AutoTuneSubMenuScreen());
        stateManager->changeState(ScreenState::PBIOS_MENU);
    } else {
        stateManager->addScreen(ScreenState::MAIN_MENU, new MainMenuScreen());
        stateManager->changeState(ScreenState::MAIN_MENU);
    }

    for (;;) {
        inputManager.update();
        Screen* activeScreen = stateManager->getActiveScreen();
        if (activeScreen) {
            InputEvent event;
            if (inputManager.wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasEnterPressed()) { event.type = InputEventType::BTN_ENTER_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasDownPressed()) { event.type = InputEventType::BTN_DOWN_PRESS; activeScreen->handleInput(event); }
            int enc_change = inputManager.getEncoderChange();
            if (enc_change != 0) { 
                event.type = enc_change > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                event.value = abs(enc_change);
                activeScreen->handleInput(event);
            }
        }

        UIRenderProps props;
        if (stateManager->getActiveScreenState() == ScreenState::LIVE_FILTER_TUNING || stateManager->getActiveScreenState() == ScreenState::PARAMETER_EDIT) {
            static_cast<LiveFilterTuningScreen*>(stateManager->getScreen(ScreenState::LIVE_FILTER_TUNING))->update();
        }
        if (activeScreen) {
            activeScreen->getRenderProps(&props);
        }
        
        uiManager->render(props);
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

/**
 * @brief --- DEFINITIVE REFACTOR: The data task is now a state machine for the tuning wizard. ---
 * This function no longer contains a single, monolithic block for auto-tuning. It now
 * uses a switch statement to execute discrete, non-blocking stages of the tuning
 * process, using the pBiosContext as the "float" to pass data between stages.
 * This is the definitive fix for the watchdog crash (Heisenbug).
 */
void dataTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("Data Task started on Core %d", xPortGetCoreID());

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();
        AutoTuningScreen* tuneScreen = static_cast<AutoTuningScreen*>(stateManager->getScreen(ScreenState::AUTO_TUNE_CHARACTERIZE_SIGNAL));

        switch (currentState) {
            case ScreenState::AUTO_TUNE_CHARACTERIZE_SIGNAL:
                if (tuneScreen && pBiosContext.selectedFilter) {
                    guidedTuningEngine.captureSignal(pBiosContext, adcManager, *tuneScreen);
                    guidedTuningEngine.characterizeSignal(pBiosContext, *tuneScreen);
                    stateManager->changeState(ScreenState::AUTO_TUNE_OPTIMIZE_HF);
                }
                break;

            case ScreenState::AUTO_TUNE_OPTIMIZE_HF:
                if (tuneScreen && pBiosContext.selectedFilter) {
                    guidedTuningEngine.optimizeHfStage(pBiosContext, *tuneScreen);
                    stateManager->changeState(ScreenState::AUTO_TUNE_OPTIMIZE_LF);
                }
                break;

            case ScreenState::AUTO_TUNE_OPTIMIZE_LF:
                if (tuneScreen && pBiosContext.selectedFilter) {
                    guidedTuningEngine.optimizeLfStage(pBiosContext, *tuneScreen);
                    stateManager->changeState(ScreenState::AUTO_TUNE_FINALIZE);
                }
                break;
            
            case ScreenState::AUTO_TUNE_FINALIZE:
                if (pBiosContext.selectedFilter) {
                    guidedTuningEngine.applyResults(pBiosContext);
                    configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), false);
                    configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), true);
                    vTaskDelay(pdMS_TO_TICKS(500)); 
                    stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
                }
                break;

            case ScreenState::LIVE_FILTER_TUNING:
            case ScreenState::PARAMETER_EDIT:
                 if (pBiosContext.selectedFilter) {
                    double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                    pBiosContext.selectedFilter->process(raw_voltage);
                 }
                break;
            
             case ScreenState::NOISE_ANALYSIS:
                // ... (other diagnostic tasks remain unchanged) ...
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void oneWireTask(void* pvParameters) { 
    for (;;) {
        tempManager.requestProbeTemperature();
        vTaskDelay(pdMS_TO_TICKS(1000));
        tempManager.update(); 
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif