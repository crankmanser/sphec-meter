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
    configManager.begin(faultHandler, sdManager);
    tempManager.begin(faultHandler);
    ina219.begin(faultHandler, i2cMutex);
    powerMonitor.begin(faultHandler, ina219, sdManager);
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
 * @brief --- DEFINITIVE FIX: Re-architected for robust mode handling. ---
 * This function is now correctly structured with two separate, mutually exclusive
 * logic blocks for NORMAL and PBIOS modes. This guarantees that the correct
 * data processing loop is always active for the current boot mode and prevents
 * regressions like the previously "dead" filter pipeline.
 */
void dataTask(void* pvParameters) {
    BootMode mode = static_cast<BootMode>(reinterpret_cast<intptr_t>(pvParameters));
    LOG_BOOT("Data Task started on Core %d", xPortGetCoreID());

    int last_cal_step = 1;
    double cal_points_mv[3];

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();
        
        // --- NORMAL MODE LOGIC ---
        if (mode == BootMode::NORMAL) {
            switch(currentState) {
                case ScreenState::PROBE_MEASUREMENT: {
                    ProbeMeasurementScreen* screen = static_cast<ProbeMeasurementScreen*>(stateManager->getActiveScreen());
                    if (screen) {
                        ProbeType active_probe = screen->getActiveProbeType();
                        uint8_t adc_index = (active_probe == ProbeType::PH) ? 0 : 1;
                        FilterManager* filter = (active_probe == ProbeType::PH) ? &phFilter : &ecFilter;
                        CalibrationManager* cal = (active_probe == ProbeType::PH) ? &phCalManager : &ecCalManager;
                        double raw_mv = adcManager.getVoltage(adc_index, ADS1118::DIFF_0_1);
                        filter->process(raw_mv);
                        double filtered_mv = filter->getFilter(1)->getFilteredValue();
                        double cal_val = cal->getCalibratedValue(filtered_mv);
                        double temp = tempManager.getProbeTemp();
                        double final_val = cal->getCompensatedValue(cal_val, temp, (active_probe == ProbeType::EC));
                        screen->updateData(final_val, temp, filter->getFilter(1)->getStabilityPercentage(), raw_mv, filtered_mv);
                        if (screen->captureWasRequested()) {
                            sdManager.mkdir("/readings");
                            StaticJsonDocument<1024> doc;
                            doc["timestamp"] = g_sessionTimestamp;
                            doc["probeType"] = (active_probe == ProbeType::PH) ? "pH" : "EC";
                            JsonObject reading = doc.createNestedObject("reading");
                            reading["value"] = final_val;
                            reading["unit"] = (active_probe == ProbeType::PH) ? "pH" : "uS";
                            reading["probeTemp"] = temp;
                            reading["stability"] = filter->getFilter(1)->getStabilityPercentage();
                            JsonObject pipeline = doc.createNestedObject("pipeline");
                            pipeline["raw_mV"] = raw_mv;
                            pipeline["filtered_mV"] = filtered_mv;
                            JsonObject cal_model_obj = doc.createNestedObject("calibrationModel");
                            cal->serializeModel(cal->getCurrentModel(), cal_model_obj);
                            JsonObject system_status = doc.createNestedObject("systemStatus");
                            system_status["ambientTemp"] = tempManager.getAmbientTemp();
                            system_status["humidity"] = tempManager.getHumidity();
                            system_status["batterySOC"] = powerMonitor.getSOC();
                            system_status["v3_3_bus"] = adcManager.getVoltage(0, ADS1118::AIN_2);
                            system_status["v5_0_bus"] = adcManager.getVoltage(1, ADS1118::AIN_2);
                            char filepath[128];
                            snprintf(filepath, sizeof(filepath), "/readings/%s_reading_%s.json", (active_probe == ProbeType::PH ? "ph" : "ec"), g_sessionTimestamp);
                            sdManager.saveJson(filepath, doc);
                            screen->clearCaptureRequest();
                        }
                    }
                    break;
                }
                case ScreenState::CALIBRATION_WIZARD: {
                    CalibrationWizardScreen* screen = static_cast<CalibrationWizardScreen*>(stateManager->getActiveScreen());
                    if (screen) {
                        ProbeType probe_type = screen->getProbeType();
                        uint8_t adc_index = (probe_type == ProbeType::PH) ? 0 : 1;
                        FilterManager* filter = (probe_type == ProbeType::PH) ? &phFilter : &ecFilter;
                        CalibrationManager* cal = (probe_type == ProbeType::PH) ? &phCalManager : &ecCalManager;
                        if (screen->isMeasuring()) {
                            filter->process(adcManager.getVoltage(adc_index, ADS1118::DIFF_0_1));
                            screen->setLiveStability(filter->getFilter(1)->getStabilityPercentage());
                            int current_step = screen->getCurrentStep();
                            if (current_step > last_cal_step) {
                                cal_points_mv[last_cal_step - 1] = filter->getFilter(1)->getFilteredValue();
                                last_cal_step = current_step;
                            }
                        } else if (screen->isCalculating()) {
                            cal->startNewCalibration();
                            double points_val[3] = {4.01, 6.86, 9.18};
                            if (probe_type == ProbeType::EC) { points_val[0] = 84; points_val[1] = 1413; points_val[2] = 12880; }
                            double temp = tempManager.getProbeTemp();
                            cal->addCalibrationPoint(cal_points_mv[0], points_val[0], temp);
                            cal->addCalibrationPoint(cal_points_mv[1], points_val[1], temp);
                            cal->addCalibrationPoint(cal_points_mv[2], points_val[2], temp);
                            double quality = cal->calculateNewModel(cal->getCurrentModel());
                            double drift = cal->getNewModel().sensorDrift;
                            screen->setResults(quality, drift);
                            last_cal_step = 1;
                        } else if (screen->saveWasRequested()) {
                            cal->acceptNewModel();
                            StaticJsonDocument<512> doc;
                            JsonObject root = doc.to<JsonObject>();
                            cal->serializeModel(cal->getCurrentModel(), root);
                            const char* filename = (probe_type == ProbeType::PH) ? "/ph_cal.json" : "/ec_cal.json";
                            sdManager.saveJson(filename, doc);
                            screen->clearSaveRequest();
                            stateManager->changeState(ScreenState::CALIBRATION_MENU);
                        }
                    }
                    break;
                }
                default: break;
            }
        }
        // --- PBIOS MODE LOGIC ---
        else if (mode == BootMode::PBIOS) {
            switch (currentState) {
                case ScreenState::LIVE_FILTER_TUNING:
                case ScreenState::PARAMETER_EDIT:
                     if (pBiosContext.selectedFilter) {
                        double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                        pBiosContext.selectedFilter->process(raw_voltage);
                     }
                    break;
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
                case ScreenState::PROBE_PROFILING: {
                    ProbeProfilingScreen* profileScreen = static_cast<ProbeProfilingScreen*>(stateManager->getScreen(ScreenState::PROBE_PROFILING));
                    if (profileScreen && profileScreen->isAnalyzing()) {
                        uint8_t adcIndex = profileScreen->getSelectedAdcIndex();
                        uint8_t adcInput = profileScreen->getSelectedAdcInput();
                        const char* filterName = profileScreen->getSelectedFilterName().c_str();
                        const char* calFileName = (adcIndex == 0) ? "/ph_cal.json" : "/ec_cal.json";
                        double live_r_std = 0.0, zero_point_drift = 0.0, cal_quality_score = 0.0;
                        char last_cal_timestamp[20] = "N/A";
                        PI_Filter hf_snapshot, lf_snapshot;
                        StaticJsonDocument<512> calDoc;
                        if (sdManager.loadJson(calFileName, calDoc)) {
                            zero_point_drift = calDoc["zpDrift"] | 0.0;
                            cal_quality_score = calDoc["quality"] | 0.0;
                            strncpy(last_cal_timestamp, calDoc["timestamp"] | "N/A", sizeof(last_cal_timestamp) -1);
                            last_cal_timestamp[sizeof(last_cal_timestamp) - 1] = '\0';
                        }
                        FilterManager tempFilterManager;
                        tempFilterManager.begin(faultHandler, configManager, "temp"); 
                        configManager.loadFilterSettings(tempFilterManager, filterName, true); 
                        hf_snapshot = *tempFilterManager.getFilter(0);
                        lf_snapshot = *tempFilterManager.getFilter(1);
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
                default: break;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
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