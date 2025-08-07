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
StateManager* pBiosStateManager = nullptr;
UIManager* uiManager = nullptr;
GuidedTuningEngine guidedTuningEngine;
HardwareTester hardwareTester; // Keep the tester instance

PBiosContext pBiosContext;
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;

// Task forward declarations
void pBiosUiTask(void* pvParameters);
void pBiosDataTask(void* pvParameters);
void oneWireTask(void* pvParameters);









void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    delay(50); 
    BootMode selected_mode = BootMode::PBIOS;
    faultHandler.begin();
    displayManager.begin(faultHandler);
    inputManager.begin(); 
    if (selected_mode == BootMode::PBIOS) {
        while(digitalRead(BTN_ENTER_PIN) == LOW) { delay(10); }
        inputManager.clearEnterButtonState();
    }
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    
    spiMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    
    LOG_BOOT("SpHEC Meter Booting... [pBIOS Mode]");
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

    xTaskCreatePinnedToCore(pBiosUiTask, "pBiosUiTask", 8192, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(pBiosDataTask, "pBiosDataTask", 8192, NULL, 2, NULL, 0);
    xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, 1, NULL);
    
    adcManager.setProbeState(0, ProbeState::ACTIVE);
    adcManager.setProbeState(1, ProbeState::ACTIVE);
    LOG_BOOT("Initialization Complete.");
}







void loop() { 
    vTaskSuspend(NULL); 
}






void pBiosUiTask(void* pvParameters) {
    LOG_BOOT("pBios UI Task started on Core %d", xPortGetCoreID());
    uiManager = new UIManager(displayManager);
    pBiosStateManager = new StateManager();
    
    pBiosStateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
    pBiosStateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
    pBiosStateManager->addScreen(ScreenState::AUTO_TUNING_ANALYSIS, new AutoTuningScreen());
    LiveFilterTuningScreen* tuningScreen = new LiveFilterTuningScreen(&adcManager, &pBiosContext, &phCalManager, &ecCalManager, &tempManager);
    pBiosStateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, tuningScreen);
    ParameterEditScreen* paramEditScreen = new ParameterEditScreen(&pBiosContext);
    pBiosStateManager->addScreen(ScreenState::PARAMETER_EDIT, paramEditScreen);
    pBiosStateManager->addScreen(ScreenState::MAINTENANCE_MENU, new MaintenanceScreen());
    pBiosStateManager->addScreen(ScreenState::SHUTDOWN_MENU, new ShutdownScreen());
    pBiosStateManager->addScreen(ScreenState::LIVE_VOLTMETER, new LiveVoltmeterScreen());
    pBiosStateManager->addScreen(ScreenState::HARDWARE_SELF_TEST, new HardwareTestScreen());
    pBiosStateManager->addScreen(ScreenState::PROBE_PROFILING, new ProbeProfilingScreen());

    pBiosStateManager->changeState(ScreenState::PBIOS_MENU);

    for (;;) {
        inputManager.update();
        Screen* activeScreen = pBiosStateManager->getActiveScreen();
        if (activeScreen) {
            InputEvent event;
            if (inputManager.wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasDownPressed()) {
                ScreenState currentState = pBiosStateManager->getActiveScreenState();
                if (currentState == ScreenState::LIVE_FILTER_TUNING) {
                    paramEditScreen->setParameterToEdit(tuningScreen->getSelectedParamName(), tuningScreen->getSelectedParamIndex());
                }
                if (currentState == ScreenState::PARAMETER_EDIT) {
                    if (pBiosContext.selectedFilter) {
                        configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str());
                    }
                }
                event.type = InputEventType::BTN_DOWN_PRESS;
                activeScreen->handleInput(event);
            }
            int enc_change = inputManager.getEncoderChange();
            if (enc_change != 0) { 
                event.type = enc_change > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                activeScreen->handleInput(event);
            }
        }

        UIRenderProps props;
        ScreenState currentState = pBiosStateManager->getActiveScreenState();
        
        // This must be called every frame for live graph updates
        if (pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING)) {
            static_cast<LiveFilterTuningScreen*>(pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING))->update();
        }

        if (currentState == ScreenState::PARAMETER_EDIT) {
            if (pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING)) {
                pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING)->getRenderProps(&props);
            }
            if (activeScreen) activeScreen->getRenderProps(&props);
        } else if (activeScreen) {
            activeScreen->getRenderProps(&props);
        }
        
        uiManager->render(props);
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}








void pBiosDataTask(void* pvParameters) {
    LOG_BOOT("pBios Data Task started on Core %d", xPortGetCoreID());
    for (;;) {
        if (!pBiosStateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = pBiosStateManager->getActiveScreenState();

 if (currentState == ScreenState::AUTO_TUNING_ANALYSIS) {
            AutoTuningScreen* screen = static_cast<AutoTuningScreen*>(pBiosStateManager->getScreen(ScreenState::AUTO_TUNING_ANALYSIS));
            if (screen && pBiosContext.selectedFilter) {
                // The proposeSettings function now handles all progress updates internally
                guidedTuningEngine.proposeSettings(
                    pBiosContext.selectedFilter, 
                    &adcManager, 
                    pBiosContext.selectedAdcIndex, 
                    pBiosContext.selectedAdcInput,
                    screen // Pass the screen pointer for progress updates
                );
                vTaskDelay(pdMS_TO_TICKS(500)); // Brief pause on completion
                pBiosStateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
            }
        }
        else if (currentState == ScreenState::LIVE_FILTER_TUNING || currentState == ScreenState::PARAMETER_EDIT) {
            if (pBiosContext.selectedFilter) {
                double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                double filtered_voltage = pBiosContext.selectedFilter->process(raw_voltage);
                double final_value = 0.0;
                float temp = tempManager.getProbeTemp();
                if (pBiosContext.selectedFilter == &phFilter) {
                    final_value = phCalManager.getCompensatedValue(phCalManager.getCalibratedValue(filtered_voltage), temp, false);
                } else if (pBiosContext.selectedFilter == &ecFilter) {
                    final_value = ecCalManager.getCompensatedValue(ecCalManager.getCalibratedValue(filtered_voltage), temp, true);
                }
                LiveFilterTuningScreen* screen = static_cast<LiveFilterTuningScreen*>(pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING));
                if(screen) screen->setCalibratedValue(final_value);
            }
        }
        else if (currentState == ScreenState::LIVE_VOLTMETER) {
            LiveVoltmeterScreen* screen = static_cast<LiveVoltmeterScreen*>(pBiosStateManager->getScreen(ScreenState::LIVE_VOLTMETER));
            if (screen && screen->isMeasuring()) {
                double raw_voltage = adcManager.getVoltage(screen->getSelectedAdcIndex(), screen->getSelectedAdcInput());
                screen->setLiveVoltage(raw_voltage);
            }
        }
        else if (currentState == ScreenState::HARDWARE_SELF_TEST) {
            HardwareTestScreen* screen = static_cast<HardwareTestScreen*>(pBiosStateManager->getScreen(ScreenState::HARDWARE_SELF_TEST));
            if (screen) {
                std::vector<TestResult> results;
                hardwareTester.runTests(sdManager, displayManager, adcManager, tempManager, results);
                screen->updateResults(results);
                bool all_passed = true;
                for(const auto& result : results) {
                    if (result.status == TestStatus::FAIL) { all_passed = false; break; }
                }
                screen->setFinalMessage(all_passed ? "All Systems OK!" : "One or more tests failed.");
                while(pBiosStateManager->getActiveScreenState() == ScreenState::HARDWARE_SELF_TEST) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
        else if (currentState == ScreenState::PROBE_PROFILING) {
            ProbeProfilingScreen* screen = static_cast<ProbeProfilingScreen*>(pBiosStateManager->getScreen(ScreenState::PROBE_PROFILING));
            if (screen && screen->isAnalyzing()) {
                // 1. Perform high-speed capture to get live R_std
                double samples[GT_SAMPLE_COUNT];
                double sum = 0.0;
                for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
                    samples[i] = adcManager.getVoltage(screen->getSelectedAdcIndex(), screen->getSelectedAdcInput());
                    sum += samples[i];
                    vTaskDelay(pdMS_TO_TICKS(2)); // ~500Hz sample rate
                }
                double mean = sum / GT_SAMPLE_COUNT;
                double sumSqDiff = 0.0;
                for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
                    sumSqDiff += (samples[i] - mean) * (samples[i] - mean);
                }
                double live_r_std = sqrt(sumSqDiff / GT_SAMPLE_COUNT);

                // 2. Load the filter parameters from the SD card
                FilterManager tempFilter;
                configManager.loadFilterSettings(tempFilter, screen->getSelectedFilterName().c_str());

                // 3. Push results to the screen
                screen->setAnalysisResults(live_r_std, *tempFilter.getFilter(0), *tempFilter.getFilter(1));
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
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