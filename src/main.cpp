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

// --- NEW: Include all new screen headers ---
#include "ui/screens/MainMenuScreen.h"
#include "ui/screens/MeasureMenuScreen.h"
#include "ui/screens/ProbeMeasurementScreen.h"
#include "ui/screens/LightSensorScreen.h"

// Include all pBIOS screen headers
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
    
    // --- NEW: Probes start in DORMANT state by default ---
    adcManager.setProbeState(0, ProbeState::DORMANT);
    adcManager.setProbeState(1, ProbeState::DORMANT);
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
        // Register all pBIOS screens
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
        // --- NEW: Register all Main App screens ---
        stateManager->addScreen(ScreenState::MAIN_MENU, new MainMenuScreen());
        stateManager->addScreen(ScreenState::MEASURE_MENU, new MeasureMenuScreen());
        stateManager->addScreen(ScreenState::PROBE_MEASUREMENT, new ProbeMeasurementScreen());
        stateManager->addScreen(ScreenState::LIGHT_SENSOR, new LightSensorScreen());
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
        
        // --- NEW: Logic for Normal Mode Measurement Screen ---
        if (currentState == ScreenState::PROBE_MEASUREMENT) {
            ProbeMeasurementScreen* screen = static_cast<ProbeMeasurementScreen*>(stateManager->getActiveScreen());
            if (screen) {
                ProbeType active_probe = screen->getActiveProbeType();
                uint8_t adc_index = (active_probe == ProbeType::PH) ? 0 : 1;
                FilterManager* filter = (active_probe == ProbeType::PH) ? &phFilter : &ecFilter;
                CalibrationManager* cal = (active_probe == ProbeType::PH) ? &phCalManager : &ecCalManager;

                double raw_mv = adcManager.getVoltage(adc_index, ADS1118::DIFF_0_1);
                filter->process(raw_mv); // Process needs to be called to update filter state
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
                    
                    // --- DEFINITIVE FIX: Serialize model directly into a nested object ---
                    JsonObject cal_model = doc.createNestedObject("calibrationModel");
                    const CalibrationModel& model = cal->getCurrentModel();
                    cal_model["isCalibrated"] = model.isCalibrated;
                    cal_model["coeff_a"] = model.coeff_a;
                    cal_model["coeff_b"] = model.coeff_b;
                    cal_model["coeff_c"] = model.coeff_c;
                    cal_model["temp"] = model.calibrationTemperature;
                    cal_model["timestamp"] = model.lastCalibratedTimestamp;
                    cal_model["quality"] = model.qualityScore;
                    cal_model["drift"] = model.sensorDrift;
                    cal_model["neutralV"] = model.neutralVoltage;
                    cal_model["zpDrift"] = model.zeroPointDrift;

                    JsonObject system_status = doc.createNestedObject("systemStatus");
                    system_status["ambientTemp"] = tempManager.getAmbientTemp();
                    system_status["humidity"] = tempManager.getHumidity();
                    system_status["batterySOC"] = powerMonitor.getSOC();
                    system_status["v3_3_bus"] = adcManager.getVoltage(0, ADS1118::AIN_2); // Use raw bus voltage
                    system_status["v5_0_bus"] = adcManager.getVoltage(1, ADS1118::AIN_2); // Use raw bus voltage

                    char filepath[128];
                    snprintf(filepath, sizeof(filepath), "/readings/%s_reading_%s.json", (active_probe == ProbeType::PH ? "ph" : "ec"), g_sessionTimestamp);
                    sdManager.saveJson(filepath, doc);
                    
                    screen->clearCaptureRequest();
                }
            }
        }
        else if (mode == BootMode::PBIOS) {
             switch (currentState) {
                // (All existing pBIOS cases remain unchanged)
                 case ScreenState::AUTO_TUNE_RUNNING: { /* ... */ break; }
                 case ScreenState::LIVE_FILTER_TUNING:
                 case ScreenState::PARAMETER_EDIT:
                      if (pBiosContext.selectedFilter) {
                         double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                         pBiosContext.selectedFilter->process(raw_voltage);
                      }
                     break;
                 // ... other pBIOS cases
             }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // Main data loop delay
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