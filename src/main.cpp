// File Path: /src/main.cpp
// MODIFIED FILE

#ifndef PIO_UNIT_TESTING
#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <FaultHandler.h>
#include <ConfigManager.h>
#include <DisplayManager.h>
#include <AdcManager.h>
#include <SdManager.h>
#include <INA219_Driver.h>
#include <PowerMonitor.h>
#include <FilterManager.h>
#include <CalibrationManager.h>
#include <TempManager.h>
#include "ui/InputManager.h"
#include "ui/StateManager.h"
#include "ui/UIManager.h"
#include "boot/boot_sequence.h"
#include "ui/screens/MainMenuScreen.h"
#include "ui/screens/pBiosMenuScreen.h"
#include "ui/screens/FilterSelectionScreen.h"
#include "ui/screens/LiveFilterTuningScreen.h"
#include "ui/screens/ParameterEditScreen.h"
#include "ui/screens/NoiseAnalysisScreen.h"
#include "ui/screens/DriftTrendingScreen.h"
#include <arduinoFFT.h>








// Global object declarations
FaultHandler faultHandler;
ConfigManager configManager;
DisplayManager displayManager;
AdcManager adcManager;
SdManager sdManager;
INA219_Driver ina219Driver;
PowerMonitor powerMonitor;
TempManager tempManager;
FilterManager phFilter, ecFilter, v3_3_Filter, v5_0_Filter;
CalibrationManager phCalManager, ecCalManager;
InputManager inputManager;
StateManager* mainStateManager = nullptr;
StateManager* pBiosStateManager = nullptr;
UIManager* uiManager = nullptr;
struct PBiosContext {
    FilterManager* selectedFilter = nullptr;
    uint8_t selectedAdcIndex = 0;
    uint8_t selectedAdcInput = 0;
};
PBiosContext pBiosContext;
SemaphoreHandle_t pBiosContextMutex = nullptr;
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;
volatile double raw_ph_voltage, filtered_ph_voltage;
double final_ph_value, final_ec_value, final_probe_temp, final_ambient_temp, final_humidity, final_v3_3_bus, final_v5_0_bus;
portMUX_TYPE sharedDataMutex = portMUX_INITIALIZER_UNLOCKED;

// Task forward declarations
void uiTask(void* pvParameters);
void i2cTask(void* pvParameters);
void oneWireTask(void* pvParameters);
void sensorTask(void* pvParameters);
void telemetryTask(void* pvParameters);
void hardwareCalTask(void* pvParameters);
void softwareCalTask(void* pvParameters);
void pBiosUiTask(void* pvParameters);
void pBiosDataTask(void* pvParameters);
String getSerialInput();













void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    delay(50); 
    BootMode selected_mode = (digitalRead(BTN_ENTER_PIN) == LOW) ? BootMode::PBIOS : BootMode::NORMAL;
    faultHandler.begin();
    displayManager.begin(faultHandler);
    inputManager.begin(); 
    if (selected_mode == BootMode::PBIOS) {
        Adafruit_SSD1306* display = displayManager.getDisplay(1);
        if(display) {
            displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
            display->clearDisplay();
            display->setTextSize(1);
            display->setTextColor(1);
            display->setCursor(10, 28);
            display->print("pBIOS Mode Selected");
            display->setCursor(10, 38);
            display->print("Release button...");
            display->display();
        }
        while(digitalRead(BTN_ENTER_PIN) == LOW) {
            delay(10); 
        }
        inputManager.clearEnterButtonState();
    }
    BootSelector bootAnimator(displayManager);
    bootAnimator.runBootAnimation();
    spiMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    
    if (selected_mode == BootMode::PBIOS) {
        LOG_BOOT("SpHEC Meter v2.11.18 Booting... [pBIOS Mode]");
        pBiosContextMutex = xSemaphoreCreateMutex();
        adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
        configManager.begin(faultHandler);
        sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
        tempManager.begin(faultHandler);
        phCalManager.begin(faultHandler);
        ecCalManager.begin(faultHandler);
        StaticJsonDocument<512> phCalDoc, ecCalDoc;
        if (sdManager.loadJson("/ph_cal.json", phCalDoc)) { 
            phCalManager.deserializeModel(phCalManager.getMutableCurrentModel(), phCalDoc); 
            LOG_BOOT("pH calibration loaded successfully.");
        } else {
            LOG_BOOT("WARNING: Could not load pH calibration file.");
        }
        if (sdManager.loadJson("/ec_cal.json", ecCalDoc)) { 
            ecCalManager.deserializeModel(ecCalManager.getMutableCurrentModel(), ecCalDoc); 
            LOG_BOOT("EC calibration loaded successfully.");
        } else {
            LOG_BOOT("WARNING: Could not load EC calibration file.");
        }
        phFilter.begin(faultHandler, configManager);
        ecFilter.begin(faultHandler, configManager);
        v3_3_Filter.begin(faultHandler, configManager);
        v5_0_Filter.begin(faultHandler, configManager);
        xTaskCreatePinnedToCore(pBiosUiTask, "pBiosUiTask", 8192, NULL, TASK_PRIORITY_HIGH, NULL, 1);
        xTaskCreatePinnedToCore(pBiosDataTask, "pBiosDataTask", 8192, NULL, TASK_PRIORITY_NORMAL, NULL, 0);
        xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
        adcManager.setProbeState(0, ProbeState::ACTIVE);
        adcManager.setProbeState(1, ProbeState::ACTIVE);

    } else { // BootMode::NORMAL
        LOG_BOOT("SpHEC Meter v2.11.18 Booting... [Normal Mode]");
        mainStateManager = new StateManager();
        uiManager = new UIManager(displayManager);
        mainStateManager->begin();
        mainStateManager->addScreen(ScreenState::MAIN_MENU, new MainMenuScreen());
        adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
        sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
        configManager.begin(faultHandler);
        phFilter.begin(faultHandler, configManager);
        ecFilter.begin(faultHandler, configManager);
        v3_3_Filter.begin(faultHandler, configManager);
        v5_0_Filter.begin(faultHandler, configManager);
        phCalManager.begin(faultHandler);
        ecCalManager.begin(faultHandler);
        tempManager.begin(faultHandler);
        ina219Driver.begin(faultHandler, i2cMutex);
        powerMonitor.begin(faultHandler, ina219Driver, sdManager);
        StaticJsonDocument<512> phCalDoc, ecCalDoc;
        if (sdManager.loadJson("/ph_cal.json", phCalDoc)) { phCalManager.deserializeModel(phCalManager.getMutableCurrentModel(), phCalDoc); }
        if (sdManager.loadJson("/ec_cal.json", ecCalDoc)) { ecCalManager.deserializeModel(ecCalManager.getMutableCurrentModel(), ecCalDoc); }
        xTaskCreatePinnedToCore(uiTask, "uiTask", 4096, NULL, TASK_PRIORITY_HIGH, NULL, 1);
        xTaskCreate(i2cTask, "i2cTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
        xTaskCreate(oneWireTask, "oneWireTask", 2048, NULL, TASK_PRIORITY_LOW, NULL);
        xTaskCreate(sensorTask, "sensorTask", 4096, NULL, TASK_PRIORITY_NORMAL, NULL);
        xTaskCreate(telemetryTask, "telemetryTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
        xTaskCreate(hardwareCalTask, "hardwareCalTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
        xTaskCreate(softwareCalTask, "softwareCalTask", 4096, NULL, TASK_PRIORITY_LOW, NULL);
        adcManager.setProbeState(0, ProbeState::ACTIVE);
        adcManager.setProbeState(1, ProbeState::ACTIVE);
    }
    LOG_BOOT("Initialization Complete.");
}










void loop() { 
    vTaskSuspend(NULL); 
}









void uiTask(void* pvParameters) {
    LOG_BOOT("UI Task started on Core %d", xPortGetCoreID());
    mainStateManager->changeState(ScreenState::MAIN_MENU);
    UIRenderProps* props = mainStateManager->getUiRenderProps();
    InputEvent event;
    for (;;) {
        inputManager.update();
        Screen* activeScreen = mainStateManager->getActiveScreen();
        if (activeScreen) {
            if (inputManager.wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasEnterPressed()) { event.type = InputEventType::BTN_ENTER_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasDownPressed()) { event.type = InputEventType::BTN_DOWN_PRESS; activeScreen->handleInput(event); }
            int encoderChange = inputManager.getEncoderChange();
            if (encoderChange != 0) { 
                event.type = encoderChange > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                event.value = encoderChange;
                activeScreen->handleInput(event);
            }
        }
        if (activeScreen && props) {
            activeScreen->getRenderProps(props);
            if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
                uiManager->render(*props);
                xSemaphoreGive(i2cMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}














void pBiosUiTask(void* pvParameters) {
    LOG_BOOT("pBios UI Task started on Core %d", xPortGetCoreID());
    uiManager = new UIManager(displayManager);
    pBiosStateManager = new StateManager();
    ParameterEditScreen* paramEditScreen = new ParameterEditScreen(&pBiosContext);
    LiveFilterTuningScreen* tuningScreen = new LiveFilterTuningScreen(&adcManager, &pBiosContext, &phCalManager, &ecCalManager, &tempManager);
    NoiseAnalysisScreen* noiseAnalysisScreen = new NoiseAnalysisScreen(&pBiosContext, &adcManager);
    DriftTrendingScreen* driftTrendingScreen = new DriftTrendingScreen(&pBiosContext, &adcManager);
    pBiosStateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
    pBiosStateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
    pBiosStateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, tuningScreen);
    pBiosStateManager->addScreen(ScreenState::PARAMETER_EDIT, paramEditScreen);
    pBiosStateManager->addScreen(ScreenState::NOISE_ANALYSIS, noiseAnalysisScreen);
    pBiosStateManager->addScreen(ScreenState::DRIFT_TRENDING, driftTrendingScreen);
    pBiosStateManager->changeState(ScreenState::PBIOS_MENU);
    for (;;) {
        inputManager.update();
        Screen* activeScreen = pBiosStateManager->getActiveScreen();
        if (activeScreen) {
            InputEvent event;
            if (inputManager.wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasEnterPressed()) { event.type = InputEventType::BTN_ENTER_PRESS; activeScreen->handleInput(event); }
            if (inputManager.wasDownPressed()) { 
                event.type = InputEventType::BTN_DOWN_PRESS;
                if (pBiosStateManager->getActiveScreenState() == ScreenState::LIVE_FILTER_TUNING) {
                     paramEditScreen->setParameterToEdit(
                        tuningScreen->getSelectedParamName(), 
                        tuningScreen->getSelectedParamIndex()
                    );
                }
                activeScreen->handleInput(event);
            }
            int enc_change = inputManager.getEncoderChange();
            if (enc_change != 0) { 
                event.type = enc_change > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                event.value = enc_change;
                activeScreen->handleInput(event);
            }
            UIRenderProps props;
            ScreenState currentState = pBiosStateManager->getActiveScreenState();
            tuningScreen->update();
            if (currentState == ScreenState::PARAMETER_EDIT) {
                tuningScreen->getRenderProps(&props);
            } else {
                activeScreen->getRenderProps(&props);
            }
            if (currentState == ScreenState::PARAMETER_EDIT) {
                paramEditScreen->getRenderProps(&props);
            }
            uiManager->render(props);
        }
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}











void pBiosDataTask(void* pvParameters) {
    LOG_BOOT("pBios Data Task started on Core %d", xPortGetCoreID());
    for (;;) {
        if (!pBiosStateManager) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        Screen* activeScreen = pBiosStateManager->getActiveScreen();
        ScreenState activeScreenType = pBiosStateManager->getActiveScreenState();
        if (activeScreenType == ScreenState::LIVE_FILTER_TUNING || activeScreenType == ScreenState::PARAMETER_EDIT) {
            if (pBiosContext.selectedFilter != nullptr) {
                double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                double filtered_voltage = pBiosContext.selectedFilter->process(raw_voltage);
                double final_value = 0.0;
                if (pBiosContext.selectedFilter == &phFilter) {
                    double calibrated_ph = phCalManager.getCalibratedValue(filtered_voltage);
                    final_value = phCalManager.getCompensatedValue(calibrated_ph, tempManager.getProbeTemp(), false);
                } else if (pBiosContext.selectedFilter == &ecFilter) {
                    double calibrated_ec = ecCalManager.getCalibratedValue(filtered_voltage);
                    final_value = ecCalManager.getCompensatedValue(calibrated_ec, tempManager.getProbeTemp(), true);
                } else {
                    final_value = filtered_voltage;
                }
                Screen* tuningScreen = pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING);
                if (tuningScreen) {
                    static_cast<LiveFilterTuningScreen*>(tuningScreen)->setCalibratedValue(final_value);
                }
            }
        } 
        else if (activeScreenType == ScreenState::NOISE_ANALYSIS) {
            NoiseAnalysisScreen* naScreen = static_cast<NoiseAnalysisScreen*>(activeScreen);
            if (naScreen && naScreen->isSampling()) {
                LOG_BOOT("Noise Analysis triggered on Core %d", xPortGetCoreID());
                std::vector<double> samples;
                samples.reserve(ANALYSIS_SAMPLE_COUNT);
                for (int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
                    samples.push_back(adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput));
                    if ((i + 1) % 16 == 0) {
                        int percent = (int)(((float)(i + 1) / ANALYSIS_SAMPLE_COUNT) * 100.0f);
                        naScreen->setSamplingProgress(percent);
                    }
                    vTaskDelay(pdMS_TO_TICKS(2)); 
                }
                naScreen->setSamplingProgress(100);
                double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
                double mean = sum / ANALYSIS_SAMPLE_COUNT;
                double min_val = *std::min_element(samples.begin(), samples.end());
                double max_val = *std::max_element(samples.begin(), samples.end());
                double pk_pk = max_val - min_val;
                double sq_sum = std::inner_product(samples.begin(), samples.end(), samples.begin(), 0.0);
                double variance = sq_sum / ANALYSIS_SAMPLE_COUNT - mean * mean;
                double std_dev = (variance > 0) ? std::sqrt(variance) : 0.0;
                naScreen->setAnalysisResults(mean, min_val, max_val, pk_pk, std_dev, samples);
                LOG_BOOT("Noise Analysis complete. Mean: %.2f mV, StdDev: %.2f mV", mean, std_dev);
            }
        }
        else if (activeScreenType == ScreenState::DRIFT_TRENDING) {
            DriftTrendingScreen* dtScreen = static_cast<DriftTrendingScreen*>(activeScreen);
            if (dtScreen && dtScreen->isSampling()) {
                LOG_BOOT("Drift Trending Analysis triggered on Core %d", xPortGetCoreID());
                int duration_sec = dtScreen->getSelectedDurationSec();
                float sample_rate_hz = (float)DRIFT_SAMPLE_COUNT / duration_sec;
                uint32_t sample_interval_ms = 1000 / sample_rate_hz;
                double vReal[DRIFT_SAMPLE_COUNT];
                double vImag[DRIFT_SAMPLE_COUNT];
                for (int i = 0; i < DRIFT_SAMPLE_COUNT; ++i) {
                    vReal[i] = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
                    vImag[i] = 0.0;
                    dtScreen->setSamplingProgress((int)(((float)(i + 1) / DRIFT_SAMPLE_COUNT) * 100.0f));
                    vTaskDelay(pdMS_TO_TICKS(sample_interval_ms));
                }
                dtScreen->setAnalyzing();
                vTaskDelay(pdMS_TO_TICKS(50));
                arduinoFFT FFT = arduinoFFT(vReal, vImag, DRIFT_SAMPLE_COUNT, sample_rate_hz);
                FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
                FFT.Compute(FFT_FORWARD);
                FFT.ComplexToMagnitude();
                dtScreen->setAnalysisResults(vReal);
                LOG_BOOT("Drift Trending Analysis complete.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}








void i2cTask(void* pvParameters) { 
    
    for (;;) 
            { 
                powerMonitor.update(); 
                vTaskDelay(pdMS_TO_TICKS(100)); 
            } 
}







void oneWireTask(void* pvParameters) { 

    for (;;) 
            { tempManager.update(); 
                vTaskDelay(pdMS_TO_TICKS(2000)); 
            }
}

String getSerialInput(){ String input = ""; 
    char c; 
    while (true) { if (Serial.available()) {
        
        c = Serial.read(); 
        if (c == '\n' || c == '\r') { 
            
            while
            (Serial.available() && (Serial.peek() == '\n' || Serial.peek() == '\r')) 
            { Serial.read(); } return input; } input += c; 
        } 
        vTaskDelay(pdMS_TO_TICKS(10)); 
    
    } return "";}







void sensorTask(void* pvParameters) { 
    for (;;) {
        double raw_ph_v = adcManager.getVoltage(0, ADS1118::DIFF_0_1);
        double temp_filtered_ph_v = phFilter.process(raw_ph_v);
        taskENTER_CRITICAL(&sharedDataMutex);
        raw_ph_voltage = raw_ph_v;
        filtered_ph_voltage = temp_filtered_ph_v;
        taskEXIT_CRITICAL(&sharedDataMutex);
        double calibrated_ph = phCalManager.getCalibratedValue(temp_filtered_ph_v);
        final_ph_value = phCalManager.getCompensatedValue(calibrated_ph, tempManager.getProbeTemp(), false);
        double raw_ec_v = adcManager.getVoltage(1, ADS1118::DIFF_0_1);
        double filtered_ec_v = ecFilter.process(raw_ec_v);
        double calibrated_ec = ecCalManager.getCalibratedValue(filtered_ec_v);
        final_ec_value = ecCalManager.getCompensatedValue(calibrated_ec, tempManager.getProbeTemp(), true);
        double raw_v3_3 = adcManager.getVoltage(0, ADS1118::AIN_2); 
        final_v3_3_bus = v3_3_Filter.process(raw_v3_3);
        double raw_v5_0 = adcManager.getVoltage(1, ADS1118::AIN_2);
        final_v5_0_bus = v5_0_Filter.process(raw_v5_0);
        final_probe_temp = tempManager.getProbeTemp();
        final_ambient_temp = tempManager.getAmbientTemp();
        final_humidity = tempManager.getHumidity();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}










void telemetryTask(void* pvParameters) { 
    for (;;) {
        double pH, ec, p_temp, a_temp, hum, v33, v50, r_ph, f_ph;
        taskENTER_CRITICAL(&sharedDataMutex);
        pH = final_ph_value; ec = final_ec_value; p_temp = final_probe_temp; a_temp = final_ambient_temp;
        hum = final_humidity; v33 = final_v3_3_bus; v50 = final_v5_0_bus;
        r_ph = raw_ph_voltage; f_ph = filtered_ph_voltage;
        taskEXIT_CRITICAL(&sharedDataMutex);
        Serial.println("\n[TELEMETRY] -----------------------------");
        Serial.printf("  Probes:\n");
        Serial.printf("    - pH:         %.2f (Raw: %.2f mV, Filtered: %.2f mV)\n", pH, r_ph, f_ph);
        Serial.printf("    - EC:         %.2f uS/cm\n", ec);
        Serial.printf("  Temperatures:\n");
        Serial.printf("    - Probe:      %.2f C\n", p_temp);
        Serial.printf("    - Ambient:    %.2f C\n", a_temp);
        Serial.printf("    - Humidity:   %.1f %%\n", hum);
        Serial.printf("  System Bus:\n");
        Serial.printf("    - 3.3V Bus:   %.2f mV\n", v33);
        Serial.printf("    - 5.0V Bus:   %.2f mV\n", v50);
        Serial.printf("  Power:\n");
        Serial.printf("    - SOC:        %.1f %%\n", powerMonitor.getSOC());
        Serial.printf("    - Current:    %.2f mA\n", powerMonitor.getCurrent());
        Serial.println("-----------------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}











void hardwareCalTask(void* pvParameters) { 
    vTaskDelay(pdMS_TO_TICKS(5500));
    Serial.println("\n\n===== Hardware Calibration Wizard =====");
    Serial.println("Press 'h' to adjust the pH hardware offset (Po).");
    String choice = getSerialInput();
    if (choice == "h") {
        Serial.println("\n--- Hardware pH Offset Adjustment ---");
        while(Serial.available() == 0) {
            double current_raw_voltage;
            taskENTER_CRITICAL(&sharedDataMutex);
            current_raw_voltage = raw_ph_voltage;
            taskEXIT_CRITICAL(&sharedDataMutex);
            Serial.printf("--> Current Raw Voltage: %.2f mV\r", current_raw_voltage);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        while (Serial.available() > 0) { Serial.read(); }
        Serial.println("\nHardware adjustment complete.");
    } else {
        Serial.println("Skipping hardware calibration.");
    }
    vTaskSuspend(NULL);
}








void softwareCalTask(void* pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    Serial.println("\n\n===== Software Calibration Wizard =====");
    Serial.println("Press 's' to start 3-point software pH calibration.");
    String choice = getSerialInput();
    if (choice == "s") {
        Serial.println("Starting Software pH Calibration...");
        phCalManager.startNewCalibration();
        const CalibrationModel& previousPhModel = phCalManager.getCurrentModel();
        for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) { }
        double quality = phCalManager.calculateNewModel(previousPhModel);
        if (quality > 0) { } 
        else { Serial.println("ERROR: Failed to calculate new model."); }
    } else {
        Serial.println("Skipping software calibration.");
    }
    vTaskSuspend(NULL);
}
#endif