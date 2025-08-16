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
#include <PowerMonitor.h>
#include <INA219_Driver.h>
#include <FilterManager.h>
#include <CalibrationManager.h>
#include "ui/InputManager.h"
#include "ui/StateManager.h"
#include "ui/UIManager.h"
#include "boot/boot_sequence.h"
#include "pBiosContext.h"
#include "GuidedTuningEngine.h"
#include "HardwareTester.h"
#include <arduinoFFT.h>

// Screen Headers
#include "ui/screens/MainMenuScreen.h"
#include "ui/screens/MeasureMenuScreen.h"
#include "ui/screens/ProbeMeasurementScreen.h"
#include "ui/screens/LightSensorScreen.h"
#include "ui/screens/CalibrationMenuScreen.h"
#include "ui/screens/CalibrationWizardScreen.h"
#include "ui/screens/ProbeHealthCheckScreen.h"
#include "ui/screens/TempCalibrationScreen.h"
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
#include "ui/screens/PowerOffScreen.h"

// Global objects
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;
TempManager tempManager;
RtcManager rtcManager;
INA219_Driver ina219;
PowerMonitor powerMonitor;
FilterManager phFilter, ecFilter, v3_3_Filter, v5_0_Filter;
CalibrationManager phCalManager, ecCalManager;
InputManager inputManager;
StateManager* stateManager = nullptr;
UIManager* uiManager = nullptr;
GuidedTuningEngine guidedTuningEngine;
HardwareTester hardwareTester;

char g_sessionTimestamp[20];
PBiosContext pBiosContext;
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;

void uiTask(void* pvParameters);
void dataTask(void* pvParameters);
void oneWireTask(void* pvParameters);
void i2cTask(void* pvParameters);

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
    displayManager.begin(faultHandler);
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    spiMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    sdManager.mkdir("/captures");
    configManager.begin(faultHandler, sdManager);
    tempManager.begin(faultHandler);
    ina219.begin(faultHandler, i2cMutex);
    powerMonitor.begin(faultHandler, ina219, sdManager);

    phFilter.begin(faultHandler, "ph_filter");
    ecFilter.begin(faultHandler, "ec_filter");
    v3_3_Filter.begin(faultHandler, "v3_3_filter");
    v5_0_Filter.begin(faultHandler, "v5_0_filter");

    phCalManager.begin(faultHandler);
    ecCalManager.begin(faultHandler);

    if (!configManager.loadFilterSettings(phFilter, "ph_filter")) {
        configManager.saveFilterSettings(phFilter, "ph_filter", "default");
    }
    if (!configManager.loadFilterSettings(ecFilter, "ec_filter")) {
        configManager.saveFilterSettings(ecFilter, "ec_filter", "default");
    }
    if (!configManager.loadFilterSettings(v3_3_Filter, "v3_3_filter")) {
        configManager.saveFilterSettings(v3_3_Filter, "v3_3_filter", "default");
    }
    if (!configManager.loadFilterSettings(v5_0_Filter, "v5_0_filter")) {
        configManager.saveFilterSettings(v5_0_Filter, "v5_0_filter", "default");
    }

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
    xTaskCreate(i2cTask, "i2cTask", 2048, NULL, 1, NULL);
    adcManager.setProbeState(0, ProbeState::DORMANT);
    adcManager.setProbeState(1, ProbeState::DORMANT);
    LOG_BOOT("Initialization Complete. Mode: %s", selected_mode == BootMode::PBIOS ? "pBIOS" : "Normal");
}

void loop() {
    vTaskSuspend(NULL);
}

void uiTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    uiManager = new UIManager(displayManager);
    stateManager = new StateManager();
    stateManager->begin();
    if (mode == BootMode::PBIOS) {
        stateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
        stateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::AUTO_TUNE_RUNNING, new AutoTuningScreen());
        stateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, new LiveFilterTuningScreen(&pBiosContext, &phCalManager, &ecCalManager, &tempManager));
        stateManager->addScreen(ScreenState::PARAMETER_EDIT, new ParameterEditScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::MAINTENANCE_MENU, new MaintenanceScreen());
        stateManager->addScreen(ScreenState::SHUTDOWN_MENU, new ShutdownScreen());
        stateManager->addScreen(ScreenState::LIVE_VOLTMETER, new LiveVoltmeterScreen());
        stateManager->addScreen(ScreenState::HARDWARE_SELF_TEST, new HardwareTestScreen());
        stateManager->addScreen(ScreenState::PROBE_PROFILING, new ProbeProfilingScreen());
        stateManager->addScreen(ScreenState::NOISE_ANALYSIS, new NoiseAnalysisScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::DRIFT_TRENDING, new DriftTrendingScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::AUTO_TUNE_SUB_MENU, new AutoTuneSubMenuScreen());
        stateManager->addScreen(ScreenState::POWER_OFF, new PowerOffScreen());
        stateManager->changeState(ScreenState::PBIOS_MENU);
    } else {
        stateManager->addScreen(ScreenState::MAIN_MENU, new MainMenuScreen());
        stateManager->addScreen(ScreenState::MEASURE_MENU, new MeasureMenuScreen());
        stateManager->addScreen(ScreenState::PROBE_MEASUREMENT, new ProbeMeasurementScreen());
        stateManager->addScreen(ScreenState::LIGHT_SENSOR, new LightSensorScreen());
        stateManager->addScreen(ScreenState::CALIBRATION_MENU, new CalibrationMenuScreen());
        stateManager->addScreen(ScreenState::CALIBRATION_WIZARD, new CalibrationWizardScreen());
        stateManager->addScreen(ScreenState::PROBE_HEALTH_CHECK, new ProbeHealthCheckScreen());
        stateManager->addScreen(ScreenState::TEMP_CALIBRATION, new TempCalibrationScreen());
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
 * @brief The main data processing task.
 * @version 3.1.13
 */
void dataTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("Data Task started on Core %d", xPortGetCoreID());

    ScreenState lastState = ScreenState::NONE;

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();
        Screen* activeScreen = stateManager->getActiveScreen();

        if (currentState != lastState) {
            adcManager.setProbeState(0, ProbeState::DORMANT);
            adcManager.setProbeState(1, ProbeState::DORMANT);

            if (mode == BootMode::NORMAL) {
                if (currentState == ScreenState::CALIBRATION_MENU) {
                    adcManager.setProbeState(0, ProbeState::ACTIVE);
                    adcManager.setProbeState(1, ProbeState::ACTIVE);
                }
                else if (currentState == ScreenState::PROBE_MEASUREMENT) {
                    ProbeMeasurementScreen* screen = static_cast<ProbeMeasurementScreen*>(activeScreen);
                    if (screen) {
                        uint8_t probe_index = (screen->getActiveProbeType() == ProbeType::PH) ? 0 : 1;
                        adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
                    }
                } else if (currentState == ScreenState::CALIBRATION_WIZARD) {
                    CalibrationWizardScreen* screen = static_cast<CalibrationWizardScreen*>(activeScreen);
                    if(screen) {
                        uint8_t probe_index = (screen->getProbeType() == ProbeType::PH) ? 0 : 1;
                        adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
                        adcManager.setProbeState(1 - probe_index, ProbeState::DORMANT); 
                    }
                }
            } else if (mode == BootMode::PBIOS) {
                 if (currentState == ScreenState::LIVE_FILTER_TUNING ||
                     currentState == ScreenState::PARAMETER_EDIT ||
                     currentState == ScreenState::NOISE_ANALYSIS ||
                     currentState == ScreenState::DRIFT_TRENDING ||
                     currentState == ScreenState::LIVE_VOLTMETER ||
                     currentState == ScreenState::PROBE_PROFILING) {
                    adcManager.setProbeState(pBiosContext.selectedAdcIndex, ProbeState::ACTIVE);
                 }
            }
            lastState = currentState;
        }

        #if DEBUG_DIAGNOSTIC_PIPELINE == 1
        static unsigned long lastLogTime = 0;
        if (millis() - lastLogTime > 1000) {
            if (currentState == ScreenState::PROBE_MEASUREMENT || (currentState == ScreenState::CALIBRATION_WIZARD && ((CalibrationWizardScreen*)activeScreen)->isMeasuring())) {
                lastLogTime = millis();
                FilterManager* filter = nullptr;
                if (currentState == ScreenState::PROBE_MEASUREMENT) {
                    filter = (((ProbeMeasurementScreen*)activeScreen)->getActiveProbeType() == ProbeType::PH) ? &phFilter : &ecFilter;
                } else {
                    filter = (((CalibrationWizardScreen*)activeScreen)->getProbeType() == ProbeType::PH) ? &phFilter : &ecFilter;
                }
                if (filter) {
                    // --- Use the new centralized method for the diagnostic log ---
                    int stability = filter->getNoiseReductionPercentage();
                    LOG_DIAG("--- Normal Boot Filter Report ---");
                    LOG_DIAG("HF Setpoints: Settle=%.3f, Smooth=%.3f", filter->getFilter(0)->settleThreshold, filter->getFilter(0)->lockSmoothing);
                    LOG_DIAG("LF Setpoints: Settle=%.3f, Smooth=%.3f", filter->getFilter(1)->settleThreshold, filter->getFilter(1)->lockSmoothing);
                    LOG_DIAG("Live Stats: Raw_std=%.4f, LF_out_std=%.4f", filter->getFilter(0)->getRawStandardDeviation(), filter->getFilter(1)->getFilteredStandardDeviation());
                    LOG_DIAG("Final Noise Reduction: %d %%", stability);
                    LOG_DIAG("---------------------------------");
                }
            }
        }
        #endif

        if (mode == BootMode::NORMAL) {
            if(currentState == ScreenState::CALIBRATION_MENU) {
                if(adcManager.isProbeActive(0)) phFilter.process(adcManager.getVoltage(0, ADS1118::DIFF_0_1));
                if(adcManager.isProbeActive(1)) ecFilter.process(adcManager.getVoltage(1, ADS1118::DIFF_0_1));
            }
            else if (currentState == ScreenState::PROBE_MEASUREMENT) {
                ProbeMeasurementScreen* screen = static_cast<ProbeMeasurementScreen*>(activeScreen);
                if (screen) {
                    ProbeType type = screen->getActiveProbeType();
                    uint8_t adc_index = (type == ProbeType::PH) ? 0 : 1;
                    FilterManager* filter = (type == ProbeType::PH) ? &phFilter : &ecFilter;
                    CalibrationManager* calManager = (type == ProbeType::PH) ? &phCalManager : &ecCalManager;
                    double raw_mv = adcManager.getVoltage(adc_index, ADS1118::DIFF_0_1);
                    double filtered_mv = filter->process(raw_mv);
                    double cal_value = calManager->getCalibratedValue(filtered_mv);
                    double temp = tempManager.getProbeTemp();
                    double final_value = calManager->getCompensatedValue(cal_value, temp, type == ProbeType::EC);
                    
                    // --- DEFINITIVE FIX: Use the new centralized method ---
                    int stability = filter->getNoiseReductionPercentage();

                    screen->updateData(final_value, temp, stability, raw_mv, filtered_mv);
                    if (screen->captureWasRequested()) {
                        StaticJsonDocument<1024> doc;
                        doc["timestamp"] = g_sessionTimestamp;
                        JsonObject reading = doc.createNestedObject("reading");
                        reading["probeType"] = (type == ProbeType::PH) ? "pH" : "EC";
                        reading["value"] = final_value;
                        reading["temperature"] = temp;
                        reading["stability"] = stability;
                        reading["raw_mV"] = raw_mv;
                        reading["filtered_mV"] = filtered_mv;
                        JsonObject system = doc.createNestedObject("system");
                        system["soc"] = powerMonitor.getSOC();
                        system["soh"] = powerMonitor.getSOH();
                        JsonObject filterSettings = doc.createNestedObject("filter_settings");
                        filterSettings["hf_settle"] = filter->getFilter(0)->settleThreshold;
                        filterSettings["lf_settle"] = filter->getFilter(1)->settleThreshold;
                        JsonObject calModel = doc.createNestedObject("calibration_model");
                        calManager->serializeModel(calManager->getCurrentModel(), calModel);
                        char filepath[64];
                        snprintf(filepath, sizeof(filepath), "/captures/capture_%s.json", g_sessionTimestamp);
                        sdManager.saveJson(filepath, doc);
                        screen->clearCaptureRequest();
                    }
                }
            }
            else if (currentState == ScreenState::CALIBRATION_WIZARD) {
                CalibrationWizardScreen* screen = static_cast<CalibrationWizardScreen*>(activeScreen);
                if (screen && screen->isMeasuring()) {
                    ProbeType type = screen->getProbeType();
                    uint8_t adc_index = (type == ProbeType::PH) ? 0 : 1;
                    FilterManager* filter = (type == ProbeType::PH) ? &phFilter : &ecFilter;
                    CalibrationManager* calManager = (type == ProbeType::PH) ? &phCalManager : &ecCalManager;
                    double raw_voltage = adcManager.getVoltage(adc_index, ADS1118::DIFF_0_1);
                    filter->process(raw_voltage);
                    
                    // --- DEFINITIVE FIX: Use the new centralized method ---
                    int stability = filter->getNoiseReductionPercentage();
                    screen->setLiveStability(stability);

                    if (screen->pointCaptureWasRequested()) {
                        double filtered_voltage = filter->getFilter(1)->getFilteredValue();
                        float temperature = tempManager.getProbeTemp();
                        double known_value = 0.0;
                        if(type == ProbeType::PH) {
                            const double ph_points[] = {4.01, 6.86, 9.18};
                            known_value = ph_points[screen->getCurrentStep() - 1];
                        } else {
                            const double ec_points[] = {0.084, 1.413, 12.88}; 
                            known_value = ec_points[screen->getCurrentStep() - 1];
                        }
                        calManager->addCalibrationPoint(filtered_voltage, known_value, temperature);
                        screen->clearPointCaptureRequest();
                        screen->advanceToNextStep();
                        if (screen->getCurrentStep() > 3) {
                            screen->transitionToCalculating();
                        }
                    }
                } else if (screen && screen->isCalculating()) {
                    ProbeType type = screen->getProbeType();
                    CalibrationManager* calManager = (type == ProbeType::PH) ? &phCalManager : &ecCalManager;
                    calManager->calculateNewModel(calManager->getCurrentModel());
                    double quality = calManager->getNewModel().qualityScore;
                    double drift = calManager->getNewModel().sensorDrift;
                    screen->setResults(quality, drift);
                } else if (screen && screen->saveWasRequested()){
                    ProbeType type = screen->getProbeType();
                    CalibrationManager* calManager = (type == ProbeType::PH) ? &phCalManager : &ecCalManager;
                    const char* filename = (type == ProbeType::PH) ? "/ph_cal.json" : "/ec_cal.json";
                    calManager->acceptNewModel();
                    StaticJsonDocument<1024> doc;
                    JsonObject root = doc.to<JsonObject>();
                    calManager->serializeModel(calManager->getCurrentModel(), root);
                    sdManager.saveJson(filename, doc);
                    screen->clearSaveRequest();
                    stateManager->changeState(ScreenState::CALIBRATION_MENU);
                }
            }
        } else if (mode == BootMode::PBIOS) {
            switch (currentState) {
                case ScreenState::LIVE_FILTER_TUNING:
                case ScreenState::PARAMETER_EDIT:
                     if (pBiosContext.selectedFilter) {
                        double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                        pBiosContext.selectedFilter->process(raw_voltage);
                     }
                    break;
                case ScreenState::AUTO_TUNE_RUNNING: {
                    AutoTuningScreen* tuneScreen = static_cast<AutoTuningScreen*>(activeScreen);
                    if (tuneScreen && pBiosContext.selectedFilter) {
                        adcManager.setProbeState(pBiosContext.selectedAdcIndex, ProbeState::ACTIVE);
                        vTaskDelay(pdMS_TO_TICKS(100)); 
                        adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput); 
                        if (guidedTuningEngine.proposeSettings(pBiosContext, adcManager, sdManager, stateManager, *tuneScreen)) {
                            configManager.saveFilterSettings(*pBiosContext.selectedFilter, pBiosContext.selectedFilterName.c_str(), g_sessionTimestamp, false);
                        }
                        tuneScreen->setProgress(100, "Finalizing...");
                        vTaskDelay(pdMS_TO_TICKS(500));
                        stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
                    }
                    break;
                }
                case ScreenState::PROBE_PROFILING: {
                    ProbeProfilingScreen* screen = static_cast<ProbeProfilingScreen*>(activeScreen);
                    if (screen && screen->isAnalyzing()) {
                        for (int i = 0; i < 50; i++) {
                             double voltage = adcManager.getVoltage(screen->getSelectedAdcIndex(), screen->getSelectedAdcInput());
                             phFilter.process(voltage);
                             screen->setProgress((i*100)/50);
                             vTaskDelay(pdMS_TO_TICKS(22));
                        }
                        FilterManager* filterToProfile = (screen->getSelectedAdcIndex() == 0) ? &phFilter : &ecFilter;
                        CalibrationManager* calManagerToUse = (screen->getSelectedAdcIndex() == 0) ? &phCalManager : &ecCalManager;
                        const CalibrationModel& model = calManagerToUse->getCurrentModel();
                        double live_r_std = filterToProfile->getFilter(0)->getRawStandardDeviation();
                        double zp_drift = model.zeroPointDrift;
                        double cal_quality = model.qualityScore;
                        char time_buf[20];
                        strftime(time_buf, sizeof(time_buf), "%Y%m%d-%H%M%S", localtime(&model.lastCalibratedTimestamp));
                        screen->setAnalysisResults(live_r_std,
                                                   *filterToProfile->getFilter(0),
                                                   *filterToProfile->getFilter(1),
                                                   zp_drift,
                                                   cal_quality,
                                                   std::string(time_buf));
                    }
                    break;
                }
                case ScreenState::NOISE_ANALYSIS: {
                    NoiseAnalysisScreen* screen = static_cast<NoiseAnalysisScreen*>(activeScreen);
                    if (screen && screen->isSampling()) {
                        std::vector<double> samples;
                        samples.reserve(ANALYSIS_SAMPLE_COUNT);
                        for (int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
                            double voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                            if (!isnan(voltage)) {
                                samples.push_back(voltage);
                            }
                            screen->setSamplingProgress((i * 100) / ANALYSIS_SAMPLE_COUNT);
                            vTaskDelay(pdMS_TO_TICKS(22));
                        }
                        if (!samples.empty()) {
                            double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
                            double mean = sum / samples.size();
                            double sq_sum = std::inner_product(samples.begin(), samples.end(), samples.begin(), 0.0);
                            double std_dev = std::sqrt(sq_sum / samples.size() - mean * mean);
                            auto minmax = std::minmax_element(samples.begin(), samples.end());
                            screen->setAnalysisResults(mean, *minmax.first, *minmax.second, *minmax.second - *minmax.first, std_dev, samples);
                        }
                    }
                    break;
                }
                case ScreenState::DRIFT_TRENDING: {
                   DriftTrendingScreen* screen = static_cast<DriftTrendingScreen*>(activeScreen);
                    if (screen && screen->isSampling()) {
                        long duration_ms = screen->getSelectedDurationSec() * 1000;
                        long start_time = millis();
                        std::vector<double> samples;
                        samples.reserve(DRIFT_SAMPLE_COUNT);
                        double fft_real[DRIFT_SAMPLE_COUNT];
                        double fft_imag[DRIFT_SAMPLE_COUNT];

                        for (int i = 0; i < DRIFT_SAMPLE_COUNT; ++i) {
                            double voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                            if (!isnan(voltage)) {
                                samples.push_back(voltage);
                            }
                            screen->setSamplingProgress((i * 100) / DRIFT_SAMPLE_COUNT);
                            vTaskDelay(pdMS_TO_TICKS(22));
                        }

                        screen->setAnalyzing();
                        vTaskDelay(pdMS_TO_TICKS(100));

                        if (samples.size() == DRIFT_SAMPLE_COUNT) {
                            double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
                            double mean = sum / samples.size();
                            for(int i=0; i<DRIFT_SAMPLE_COUNT; ++i) {
                                fft_real[i] = samples[i] - mean;
                                fft_imag[i] = 0.0;
                            }
                            arduinoFFT fft(fft_real, fft_imag, DRIFT_SAMPLE_COUNT, 1000.0 / 22.0);
                            fft.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
                            fft.Compute(FFT_FORWARD);
                            fft.ComplexToMagnitude();
                            screen->setAnalysisResults(fft_real);
                        }
                    }
                    break;
                }
                default: break;
            }
        }

        if (currentState == ScreenState::LIVE_FILTER_TUNING ||
            currentState == ScreenState::PARAMETER_EDIT ||
            currentState == ScreenState::PROBE_MEASUREMENT ||
            currentState == ScreenState::NOISE_ANALYSIS ||
            currentState == ScreenState::DRIFT_TRENDING ||
            currentState == ScreenState::PROBE_PROFILING ||
            currentState == ScreenState::CALIBRATION_MENU ||
            currentState == ScreenState::CALIBRATION_WIZARD) {
            vTaskDelay(pdMS_TO_TICKS(22)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); 
        }
    }
}

void i2cTask(void *pvParameters) {
    for(;;) {
        powerMonitor.update();
        vTaskDelay(pdMS_TO_TICKS(1000));
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