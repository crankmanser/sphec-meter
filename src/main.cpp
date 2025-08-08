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
StateManager* stateManager = nullptr; // A single, unified state manager
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

/**
 * @brief --- DEFINITIVE FIX: The complete and correct setup function. ---
 * This function now implements the robust, legacy-inspired boot selection
 * logic. It performs a simple, reliable digitalRead() at power-on to determine
 * the boot mode before any complex initialization occurs. This resolves the
 * issue where the device was always booting into pBIOS.
 */
void setup() {
    Serial.begin(115200);
    delay(100);

    // 1. Determine Boot Mode
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    delay(50); 
    BootMode selected_mode = (digitalRead(BTN_ENTER_PIN) == LOW) ? BootMode::PBIOS : BootMode::NORMAL;

    // 2. Initialize Core Systems
    faultHandler.begin();
    displayManager.begin(faultHandler);
    
    // 3. Run Boot Animation
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    
    // 4. Initialize Hardware Buses & Managers
    spiMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    
    adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    configManager.begin(faultHandler, sdManager);
    tempManager.begin(faultHandler);

    // 5. Initialize Filter & Calibration Cabinets
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

    // 6. Initialize Input Manager and clear any latent presses from boot combo
    inputManager.begin(); 
    if (selected_mode == BootMode::PBIOS) {
        while(digitalRead(BTN_ENTER_PIN) == LOW) { delay(10); }
        inputManager.clearEnterButtonState();
    }

    // 7. Create RTOS Tasks
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
    
    if (mode == BootMode::PBIOS) {
        stateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
        stateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::AUTO_TUNING_ANALYSIS, new AutoTuningScreen());
        stateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, new LiveFilterTuningScreen(&adcManager, &pBiosContext, &phCalManager, &ecCalManager, &tempManager));
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
        if (stateManager->getActiveScreenState() == ScreenState::LIVE_FILTER_TUNING) {
            static_cast<LiveFilterTuningScreen*>(stateManager->getScreen(ScreenState::LIVE_FILTER_TUNING))->update();
        }
        if (activeScreen) {
            activeScreen->getRenderProps(&props);
        }
        
        uiManager->render(props);
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void dataTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("Data Task started on Core %d", xPortGetCoreID());

    if (mode == BootMode::NORMAL) {
        for(;;) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();

        if (currentState == ScreenState::AUTO_TUNING_ANALYSIS) {
            AutoTuningScreen* screen = static_cast<AutoTuningScreen*>(stateManager->getScreen(ScreenState::AUTO_TUNING_ANALYSIS));
            if (screen && pBiosContext.selectedFilter) {
                guidedTuningEngine.proposeSettings(pBiosContext.selectedFilter, &adcManager, pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput, screen);
                configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), false);
                configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), true);
                vTaskDelay(pdMS_TO_TICKS(500));
                stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
            }
        }
        else if (currentState == ScreenState::LIVE_FILTER_TUNING) {
             if (pBiosContext.selectedFilter) {
                double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                pBiosContext.selectedFilter->process(raw_voltage);
             }
        }
        else if (currentState == ScreenState::NOISE_ANALYSIS) {
            NoiseAnalysisScreen* screen = static_cast<NoiseAnalysisScreen*>(stateManager->getScreen(ScreenState::NOISE_ANALYSIS));
            if (screen && screen->isSampling()) {
                std::vector<double> samples;
                samples.reserve(ANALYSIS_SAMPLE_COUNT);
                double sum = 0.0, min_val = 9999.0, max_val = -9999.0;
                for (int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
                    double v = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                    samples.push_back(v); sum += v; if (v < min_val) min_val = v; if (v > max_val) max_val = v;
                    screen->setSamplingProgress((i * 100) / ANALYSIS_SAMPLE_COUNT); vTaskDelay(pdMS_TO_TICKS(2));
                }
                double mean = sum / ANALYSIS_SAMPLE_COUNT;
                double sumSqDiff = 0.0; for(const auto& s : samples) sumSqDiff += (s - mean) * (s - mean);
                double std_dev = sqrt(sumSqDiff / ANALYSIS_SAMPLE_COUNT);
                screen->setAnalysisResults(mean, min_val, max_val, max_val - min_val, std_dev, samples);
            }
        }
        else if (currentState == ScreenState::DRIFT_TRENDING) {
            DriftTrendingScreen* screen = static_cast<DriftTrendingScreen*>(stateManager->getScreen(ScreenState::DRIFT_TRENDING));
            if (screen && screen->isSampling()) {
                double* samples = new double[DRIFT_SAMPLE_COUNT];
                long sleep_per_sample = (screen->getSelectedDurationSec() * 1000) / DRIFT_SAMPLE_COUNT;
                for (int i = 0; i < DRIFT_SAMPLE_COUNT; ++i) {
                    samples[i] = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                    screen->setSamplingProgress((i * 100) / DRIFT_SAMPLE_COUNT); vTaskDelay(pdMS_TO_TICKS(sleep_per_sample));
                }
                screen->setAnalyzing();
                double fft_mags[FFT_BIN_COUNT] = {0};
                screen->setAnalysisResults(fft_mags);
                delete[] samples;
            }
        }
        else if (currentState == ScreenState::LIVE_VOLTMETER) {
            LiveVoltmeterScreen* screen = static_cast<LiveVoltmeterScreen*>(stateManager->getScreen(ScreenState::LIVE_VOLTMETER));
            if (screen && screen->isMeasuring()) {
                double raw_voltage = adcManager.getVoltage(screen->getSelectedAdcIndex(), screen->getSelectedAdcInput());
                screen->setLiveVoltage(raw_voltage);
            }
        }
        else if (currentState == ScreenState::HARDWARE_SELF_TEST) {
            HardwareTestScreen* screen = static_cast<HardwareTestScreen*>(stateManager->getScreen(ScreenState::HARDWARE_SELF_TEST));
            if (screen) {
                std::vector<TestResult> results;
                hardwareTester.runTests(sdManager, displayManager, adcManager, tempManager, results);
                screen->updateResults(results);
                bool all_passed = true; for(const auto& result : results) { if (result.status == TestStatus::FAIL) { all_passed = false; break; } }
                screen->setFinalMessage(all_passed ? "All Systems OK!" : "One or more tests failed.");
                while(stateManager->getActiveScreenState() == ScreenState::HARDWARE_SELF_TEST) { vTaskDelay(pdMS_TO_TICKS(100)); }
            }
        }
        else if (currentState == ScreenState::PROBE_PROFILING) {
            ProbeProfilingScreen* screen = static_cast<ProbeProfilingScreen*>(stateManager->getScreen(ScreenState::PROBE_PROFILING));
            if (screen && screen->isAnalyzing()) {
                double samples[GT_SAMPLE_COUNT];
                double sum = 0.0;
                for (int i = 0; i < GT_SAMPLE_COUNT; ++i) {
                    samples[i] = adcManager.getVoltage(screen->getSelectedAdcIndex(), screen->getSelectedAdcInput());
                    sum += samples[i]; vTaskDelay(pdMS_TO_TICKS(2));
                }
                double mean = sum / GT_SAMPLE_COUNT;
                double sumSqDiff = 0.0; for (int i = 0; i < GT_SAMPLE_COUNT; ++i) { sumSqDiff += (samples[i] - mean) * (samples[i] - mean); }
                double live_r_std = sqrt(sumSqDiff / GT_SAMPLE_COUNT);
                FilterManager tempFilter;
                tempFilter.begin(faultHandler, configManager, screen->getSelectedFilterName().c_str());
                screen->setAnalysisResults(live_r_std, *tempFilter.getFilter(0), *tempFilter.getFilter(1));
            }
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