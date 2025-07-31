// File Path: /src/main.cpp
// MODIFIED FILE

#ifndef PIO_UNIT_TESTING
#include <Arduino.h>
#include "ProjectConfig.h"
#include "DebugConfig.h"
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

// --- Global object declarations for core cabinets ---
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

// --- UI Engine objects (conditionally initialized) ---
InputManager* inputManager = nullptr;
StateManager* mainStateManager = nullptr;
StateManager* pBiosStateManager = nullptr;
UIManager* uiManager = nullptr;

// --- pBIOS Shared Context ---
struct PBiosContext {
    FilterManager* selectedFilter = nullptr;
    uint8_t selectedAdcIndex = 0;
    uint8_t selectedAdcInput = 0;
};
PBiosContext pBiosContext;
SemaphoreHandle_t pBiosContextMutex = nullptr;

// --- Global variables & RTOS primitives ---
SPIClass* vspi = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;
volatile double raw_ph_voltage, filtered_ph_voltage;
double final_ph_value, final_ec_value, final_probe_temp, final_ambient_temp, final_humidity, final_v3_3_bus, final_v5_0_bus;
portMUX_TYPE sharedDataMutex = portMUX_INITIALIZER_UNLOCKED;

// --- Task forward declarations ---
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

/**
 * @brief The main setup function, run once on boot.
 * @details Implements the reboot-based dual-boot system. It checks RTC
 * memory for a boot flag. If the flag is invalid (first boot), it runs the
 * boot selector UI. Otherwise, it boots directly into the selected mode.
 */


void setup() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to connect
    printf("\n\n--- NEW BOOT ---\n");
    
    // --- DEBUG LOG ---
    // Read the boot mode from RTC memory and print its raw integer value.
    // This will tell us if the value survived the reboot.
    BootMode selected_mode = rtc_boot_mode;
    printf("[SETUP] Value read from RTC memory: %d (NORMAL=0, PBIOS=1)\n", (int)selected_mode);

    // --- Initialize hardware that is common to all modes ---
    spiMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    vspi = new SPIClass(VSPI);
    vspi->begin(VSPI_SCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    faultHandler.begin();
    displayManager.begin(faultHandler);

    // --- Main Boot Logic ---
    if (selected_mode != BootMode::NORMAL && selected_mode != BootMode::PBIOS) {
        // --- First Boot or Unspecified: Show the Boot Selector UI ---
        LOG_BOOT("SpHEC Meter v2.11.4 Booting... [First Time Setup]");
        BootSelector bootSelector(displayManager);
        bootSelector.runBootSequence(2000);

    } else {
        // A valid mode was selected on the previous boot.
        rtc_boot_mode = BootMode::NORMAL; // Clear the one-time flag

        if (selected_mode == BootMode::PBIOS) {
            // --- Boot directly into pBIOS mode ---
            // (This block is unchanged)
            LOG_BOOT("SpHEC Meter v2.11.4 Booting... [pBIOS Mode]");
            pBiosContextMutex = xSemaphoreCreateMutex();
            adcManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN);
            configManager.begin(faultHandler);
            phFilter.begin(faultHandler, configManager);
            ecFilter.begin(faultHandler, configManager);
            v3_3_Filter.begin(faultHandler, configManager);
            v5_0_Filter.begin(faultHandler, configManager);
            xTaskCreatePinnedToCore(pBiosUiTask, "pBiosUiTask", 4096, NULL, TASK_PRIORITY_HIGH, NULL, 1);
            xTaskCreatePinnedToCore(pBiosDataTask, "pBiosDataTask", 4096, NULL, TASK_PRIORITY_NORMAL, NULL, 0);
            adcManager.setProbeState(0, ProbeState::ACTIVE);
            adcManager.setProbeState(1, ProbeState::ACTIVE);
        } else { // selected_mode == BootMode::NORMAL
            // --- Boot directly into Normal Application mode ---
            // (This block is unchanged)
            LOG_BOOT("SpHEC Meter v2.11.4 Booting... [Normal Mode]");
            inputManager = new InputManager();
            mainStateManager = new StateManager();
            uiManager = new UIManager(displayManager);
            inputManager->begin();
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
        inputManager->update();
        Screen* activeScreen = mainStateManager->getActiveScreen();
        if (activeScreen) {
            if (inputManager->wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager->wasEnterPressed()) { event.type = InputEventType::BTN_ENTER_PRESS; activeScreen->handleInput(event); }
            if (inputManager->wasDownPressed()) { event.type = InputEventType::BTN_DOWN_PRESS; activeScreen->handleInput(event); }
            int encoderChange = inputManager->getEncoderChange();
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
    inputManager = new InputManager();
    uiManager = new UIManager(displayManager);
    pBiosStateManager = new StateManager();
    inputManager->begin();
    
    // --- FIX: Pass the shared context to the screen constructors ---
    pBiosStateManager->addScreen(ScreenState::PBIOS_MENU, new pBiosMenuScreen());
    pBiosStateManager->addScreen(ScreenState::FILTER_SELECTION, new FilterSelectionScreen(&pBiosContext));
    pBiosStateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, new LiveFilterTuningScreen(&adcManager, &pBiosContext));
    
    pBiosStateManager->changeState(ScreenState::PBIOS_MENU);

    for (;;) {
        inputManager->update();
        Screen* activeScreen = pBiosStateManager->getActiveScreen();
        if (activeScreen) {
            InputEvent event;
            if (inputManager->wasBackPressed()) { event.type = InputEventType::BTN_BACK_PRESS; activeScreen->handleInput(event); }
            if (inputManager->wasEnterPressed()) { event.type = InputEventType::BTN_ENTER_PRESS; activeScreen->handleInput(event); }
            if (inputManager->wasDownPressed()) { event.type = InputEventType::BTN_DOWN_PRESS; activeScreen->handleInput(event); }
            int enc_change = inputManager->getEncoderChange();
            if (enc_change != 0) { 
                event.type = enc_change > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                event.value = enc_change;
                activeScreen->handleInput(event);
            }
            UIRenderProps props;
            activeScreen->getRenderProps(&props);
            uiManager->render(props);
        }
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void pBiosDataTask(void* pvParameters) {
    LOG_BOOT("pBios Data Task started on Core %d", xPortGetCoreID());
    for (;;) {
        if (pBiosStateManager && pBiosStateManager->getActiveScreenState() == ScreenState::LIVE_FILTER_TUNING) {
            Screen* screen = pBiosStateManager->getActiveScreen();
            if (screen) {
                static_cast<LiveFilterTuningScreen*>(screen)->update();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void i2cTask(void* pvParameters) { for (;;) { powerMonitor.update(); vTaskDelay(pdMS_TO_TICKS(100)); } }
void oneWireTask(void* pvParameters) { for (;;) { tempManager.update(); vTaskDelay(pdMS_TO_TICKS(2000)); } }
String getSerialInput(){ String input = ""; char c; while (true) { if (Serial.available()) { c = Serial.read(); if (c == '\n' || c == '\r') { while(Serial.available() && (Serial.peek() == '\n' || Serial.peek() == '\r')) { Serial.read(); } return input; } input += c; } vTaskDelay(pdMS_TO_TICKS(10)); } return "";}

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
#endif // PIO_UNIT_TESTING