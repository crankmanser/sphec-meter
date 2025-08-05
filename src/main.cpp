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
#include "ui/screens/pBiosMenuScreen.h"
#include "ui/screens/FilterSelectionScreen.h"
#include "ui/screens/LiveFilterTuningScreen.h"
#include "ui/screens/ParameterEditScreen.h"
#include "pBiosContext.h"

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
    
    // --- MODIFIED: Pass SdManager to ConfigManager ---
    sdManager.begin(faultHandler, vspi, spiMutex, SD_CS_PIN, ADC1_CS_PIN, ADC2_CS_PIN);
    configManager.begin(faultHandler, sdManager);
    
    tempManager.begin(faultHandler);

    // --- MODIFIED: Pass names to FilterManager begin() ---
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
    LiveFilterTuningScreen* tuningScreen = new LiveFilterTuningScreen(&adcManager, &pBiosContext, &phCalManager, &ecCalManager, &tempManager);
    pBiosStateManager->addScreen(ScreenState::LIVE_FILTER_TUNING, tuningScreen);
    ParameterEditScreen* paramEditScreen = new ParameterEditScreen(&pBiosContext);
    pBiosStateManager->addScreen(ScreenState::PARAMETER_EDIT, paramEditScreen);
    
    pBiosStateManager->changeState(ScreenState::PBIOS_MENU);

    for (;;) {
        inputManager.update();
        Screen* activeScreen = pBiosStateManager->getActiveScreen();
        if (activeScreen) {
            InputEvent event;

            // --- Handle Back Button ---
            if (inputManager.wasBackPressed()) { 
                event.type = InputEventType::BTN_BACK_PRESS; 
                activeScreen->handleInput(event); 
            }

            // --- Handle Down/Action Button ---
            if (inputManager.wasDownPressed()) {
                ScreenState currentState = pBiosStateManager->getActiveScreenState();

                // If on tuning screen, prepare the edit screen
                if (currentState == ScreenState::LIVE_FILTER_TUNING) {
                    paramEditScreen->setParameterToEdit(
                        tuningScreen->getSelectedParamName(),
                        tuningScreen->getSelectedParamIndex()
                    );
                }
                
                // --- NEW: Save settings when "Set" is pressed on the edit screen ---
                if (currentState == ScreenState::PARAMETER_EDIT) {
                    if (pBiosContext.selectedFilter) {
                        configManager.saveFilterSettings(
                            *pBiosContext.selectedFilter, 
                            pBiosContext.selectedFilterName.c_str()
                        );
                        // Optional: Add some visual feedback here
                    }
                }

                event.type = InputEventType::BTN_DOWN_PRESS;
                activeScreen->handleInput(event);
            }
            
            // --- Handle Encoder ---
            int enc_change = inputManager.getEncoderChange();
            if (enc_change != 0) { 
                event.type = enc_change > 0 ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                activeScreen->handleInput(event);
            }
        }

        // --- Update and Render Logic (Unchanged) ---
        tuningScreen->update();

        UIRenderProps props;
        ScreenState currentState = pBiosStateManager->getActiveScreenState();

        if (currentState == ScreenState::PARAMETER_EDIT) {
            tuningScreen->getRenderProps(&props);
            paramEditScreen->getRenderProps(&props);
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
        if (pBiosContext.selectedFilter != nullptr) {
            double raw_voltage = adcManager.getVoltage(pBiosContext.selectedAdcIndex, pBiosContext.selectedAdcInput);
            double filtered_voltage = pBiosContext.selectedFilter->process(raw_voltage);
            double final_value = 0.0;
            float temp = tempManager.getProbeTemp();

            if (pBiosContext.selectedFilter == &phFilter) {
                double calibrated_ph = phCalManager.getCalibratedValue(filtered_voltage);
                final_value = phCalManager.getCompensatedValue(calibrated_ph, temp, false);
            } else if (pBiosContext.selectedFilter == &ecFilter) {
                double calibrated_ec = ecCalManager.getCalibratedValue(filtered_voltage);
                final_value = ecCalManager.getCompensatedValue(calibrated_ec, temp, true);
            }

            Screen* screen = pBiosStateManager->getScreen(ScreenState::LIVE_FILTER_TUNING);
            if(screen) {
                static_cast<LiveFilterTuningScreen*>(screen)->setCalibratedValue(final_value);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void oneWireTask(void* pvParameters) { 
    for (;;) {
        tempManager.update(); 
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}

#endif