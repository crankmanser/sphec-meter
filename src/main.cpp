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
        stateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, new LiveFilterTuningScreen(&pBiosContext, &phCalManager, &ecCalManager, &tempManager));
        stateManager->addScreen(ScreenState::PARAMETER_EDIT, new ParameterEditScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::MAINTENANCE_MENU, new MaintenanceScreen());
        stateManager->addScreen(ScreenState::SHUTDOWN_MENU, new ShutdownScreen());
        stateManager->addScreen(ScreenState::LIVE_VOLTMETER, new LiveVoltmeterScreen());
        stateManager->addScreen(ScreenState::HARDWARE_SELF_TEST, new HardwareTestScreen());
        stateManager->addScreen(ScreenState::PROBE_PROFILING, new ProbeProfilingScreen());
        stateManager->addScreen(ScreenState::NOISE_ANALYSIS, new NoiseAnalysisScreen(&pBiosContext));
        stateManager->addScreen(ScreenState::DRIFT_TRENDING, new DriftTrendingScreen(&pBiosContext));
        // --- DEFINITIVE FIX: Update constructor call to match new signature ---
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

    ScreenState lastState = ScreenState::NONE;

    for (;;) {
        if (!stateManager) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }
        ScreenState currentState = stateManager->getActiveScreenState();

        if (currentState != lastState) {
            adcManager.setProbeState(0, ProbeState::DORMANT);
            adcManager.setProbeState(1, ProbeState::DORMANT);

            if (mode == BootMode::NORMAL) {
                if (currentState == ScreenState::PROBE_MEASUREMENT) {
                    ProbeMeasurementScreen* screen = static_cast<ProbeMeasurementScreen*>(stateManager->getActiveScreen());
                    if (screen) {
                        uint8_t probe_index = (screen->getActiveProbeType() == ProbeType::PH) ? 0 : 1;
                        adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
                    }
                } else if (currentState == ScreenState::CALIBRATION_WIZARD) {
                    CalibrationWizardScreen* screen = static_cast<CalibrationWizardScreen*>(stateManager->getActiveScreen());
                    if(screen) {
                        uint8_t probe_index = (screen->getProbeType() == ProbeType::PH) ? 0 : 1;
                        adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
                    }
                }
            } else if (mode == BootMode::PBIOS) {
                 if (currentState == ScreenState::LIVE_FILTER_TUNING ||
                     currentState == ScreenState::PARAMETER_EDIT ||
                     currentState == ScreenState::NOISE_ANALYSIS ||
                     currentState == ScreenState::DRIFT_TRENDING ||
                     currentState == ScreenState::LIVE_VOLTMETER) {
                    adcManager.setProbeState(pBiosContext.selectedAdcIndex, ProbeState::ACTIVE);
                 }
            }
            lastState = currentState;
        }

        if (mode == BootMode::NORMAL) {
            // ... (Normal mode logic) ...
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
                    AutoTuningScreen* tuneScreen = static_cast<AutoTuningScreen*>(stateManager->getScreen(ScreenState::AUTO_TUNE_RUNNING));
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
                    // ... (Probe profiling logic) ...
                    break;
                }
                case ScreenState::NOISE_ANALYSIS: {
                    // ... (Noise analysis logic) ...
                    break;
                }
                case ScreenState::DRIFT_TRENDING: {
                   // ... (Drift trending logic) ...
                    break;
                }
                default: break;
            }
        }

        if (currentState == ScreenState::LIVE_FILTER_TUNING ||
            currentState == ScreenState::PARAMETER_EDIT ||
            currentState == ScreenState::PROBE_MEASUREMENT) {
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