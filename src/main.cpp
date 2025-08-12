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
#include <RtcManager.h>
#include <FilterManager.h>
#include <CalibrationManager.h>
#include "ui/InputManager.h"
#include "ui/StateManager.h"
#include "ui/UIManager.h"
#include "boot/boot_sequence.h"
#include "pBiosContext.h"
#include "GuidedTuningEngine.h"
#include "HardwareTester.h"

// Include all screen headers
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
RtcManager rtcManager;
FilterManager phFilter, ecFilter, v3_3_Filter, v5_0_Filter;
CalibrationManager phCalManager, ecCalManager;
InputManager inputManager;
StateManager* stateManager = nullptr;
UIManager* uiManager = nullptr;
GuidedTuningEngine guidedTuningEngine;
HardwareTester hardwareTester;

// Global session timestamp
char g_sessionTimestamp[20];

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
    
    i2cMutex = xSemaphoreCreateMutex();
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    rtcManager.begin(faultHandler, i2cMutex);
    rtcManager.getTimestamp(g_sessionTimestamp, sizeof(g_sessionTimestamp));
    LOG_BOOT("Session Timestamp: %s", g_sessionTimestamp);

    displayManager.begin(faultHandler);
    
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    
    spiMutex = xSemaphoreCreateMutex();
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
        stateManager->addScreen(ScreenState::AUTO_TUNE_RUNNING, new AutoTuningScreen());
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


void dataTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("Data Task started on Core %d", xPortGetCoreID());

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();
        
        switch (currentState) {
            case ScreenState::AUTO_TUNE_RUNNING: {
                AutoTuningScreen* tuneScreen = static_cast<AutoTuningScreen*>(stateManager->getScreen(ScreenState::AUTO_TUNE_RUNNING));
                if (tuneScreen && pBiosContext.selectedFilter) {
                    if (guidedTuningEngine.proposeSettings(pBiosContext, adcManager, sdManager, stateManager, *tuneScreen)) {
                        configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), g_sessionTimestamp, false);
                    }
                    tuneScreen->setProgress(100, "Finalizing...");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
                }
                break;
            }

            case ScreenState::LIVE_FILTER_TUNING:
            case ScreenState::PARAMETER_EDIT:
                 if (pBiosContext.selectedFilter) {
                    double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                    pBiosContext.selectedFilter->process(raw_voltage);
                 }
                break;
            
            case ScreenState::PROBE_PROFILING: {
                ProbeProfilingScreen* profileScreen = static_cast<ProbeProfilingScreen*>(stateManager->getScreen(ScreenState::PROBE_PROFILING));
                // --- DEFINITIVE FIX: The backend logic is now correctly implemented ---
                if (profileScreen && profileScreen->isAnalyzing()) {
                    // 1. Get identifiers for the selected probe
                    uint8_t adcIndex = profileScreen->getSelectedAdcIndex();
                    uint8_t adcInput = profileScreen->getSelectedAdcInput();
                    const char* filterName = profileScreen->getSelectedFilterName().c_str();
                    const char* calFileName = (adcIndex == 0) ? "/ph_cal.json" : "/ec_cal.json";

                    // 2. Initialize variables for the report card
                    double live_r_std = 0.0, zero_point_drift = 0.0, cal_quality_score = 0.0;
                    char last_cal_timestamp[20] = "N/A";
                    PI_Filter hf_snapshot, lf_snapshot;

                    // 3. Load historical calibration KPIs
                    StaticJsonDocument<512> calDoc;
                    if (sdManager.loadJson(calFileName, calDoc)) {
                        zero_point_drift = calDoc["zpDrift"] | 0.0;
                        cal_quality_score = calDoc["quality"] | 0.0;
                        strncpy(last_cal_timestamp, calDoc["timestamp"] | "N/A", sizeof(last_cal_timestamp) -1);
                        last_cal_timestamp[sizeof(last_cal_timestamp) - 1] = '\0'; // Ensure null termination
                    }

                    // 4. Load saved filter parameters ("Filter Creep")
                    FilterManager tempFilterManager;
                    tempFilterManager.begin(faultHandler, configManager, "temp"); 
                    configManager.loadFilterSettings(tempFilterManager, filterName, true); 
                    hf_snapshot = *tempFilterManager.getFilter(0);
                    lf_snapshot = *tempFilterManager.getFilter(1);

                    // 5. Perform live signal analysis for R_std
                    const int num_samples = 100;
                    double sum = 0.0, sum_sq = 0.0;
                    for (int i = 0; i < num_samples; ++i) {
                        double voltage = adcManager.getVoltage(adcIndex, adcInput);
                        sum += voltage;
                        sum_sq += voltage * voltage;
                        vTaskDelay(pdMS_TO_TICKS(5));
                    }
                    double mean = sum / num_samples;
                    double variance = (sum_sq / num_samples) - (mean * mean);
                    live_r_std = (variance > 0) ? sqrt(variance) : 0.0;
                    
                    // 6. Send the completed report card to the UI
                    profileScreen->setAnalysisResults(live_r_std, hf_snapshot, lf_snapshot, zero_point_drift, cal_quality_score, std::string(last_cal_timestamp));
                }
                break;
            }
            
            case ScreenState::NOISE_ANALYSIS: {
                NoiseAnalysisScreen* noiseScreen = static_cast<NoiseAnalysisScreen*>(stateManager->getScreen(ScreenState::NOISE_ANALYSIS));
                if (noiseScreen && noiseScreen->isSampling()) {
                    uint8_t adcIndex = pBiosContext.selectedAdcIndex;
                    uint8_t adcInput = pBiosContext.selectedAdcInput;

                    std::vector<double> samples;
                    samples.reserve(ANALYSIS_SAMPLE_COUNT);
                    double min_val = 99999.0, max_val = -99999.0, sum = 0.0, sum_sq = 0.0;
                    
                    for (int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
                        double voltage = adcManager.getVoltage(adcIndex, adcInput);
                        samples.push_back(voltage);
                        sum += voltage;
                        sum_sq += voltage * voltage;
                        if (voltage < min_val) min_val = voltage;
                        if (voltage > max_val) max_val = voltage;
                        
                        if (i % (ANALYSIS_SAMPLE_COUNT / 10) == 0) {
                            noiseScreen->setSamplingProgress((i * 100) / ANALYSIS_SAMPLE_COUNT);
                        }
                        vTaskDelay(pdMS_TO_TICKS(2)); 
                    }
                    noiseScreen->setSamplingProgress(100);

                    double mean = sum / ANALYSIS_SAMPLE_COUNT;
                    double variance = (sum_sq / ANALYSIS_SAMPLE_COUNT) - (mean * mean);
                    double std_dev = (variance > 0) ? sqrt(variance) : 0.0;
                    double pk_pk = max_val - min_val;

                    noiseScreen->setAnalysisResults(mean, min_val, max_val, pk_pk, std_dev, samples);
                }
                break;
            }
            
            default:
                // No data processing needed for other states
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