// src/app/modes/pbios.cpp
// MODIFIED FILE
#include "pbios.h"
#include "presentation/DisplayManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "app/StateManager.h"
#include "presentation/UIManager.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include "managers/rtc/RtcManager.h" // <<< ADDED
#include "presentation/screens/main_menu/diagnostics/DiagnosticsMenuScreen.h" // <<< ADDED
#include "presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h" // <<< ADDED
#include "DebugMacros.h"

// External declarations for objects created in main.cpp
extern DisplayManager* displayManager;
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;
extern StateManager* stateManager;
extern UIManager* uiManager;
extern NoiseAnalysisManager* noiseAnalysisManager;
extern RtcManager* rtcManager;

void run_pbios_mode() {
    LOG_MAIN("--- Entering pBios (DIAGNOSTICS) Mode ---\n");

    // --- STAGE 1: INITIALIZE REQUIRED MANAGERS ---
    buttonManager->begin();
    encoderManager->begin();
    uiManager->begin();

    // --- STAGE 2: SETUP DIAGNOSTICS SCREEN ---
    stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(noiseAnalysisManager));
    stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
    stateManager->begin();

    LOG_MAIN("pBios initialization complete. Starting main loop.\n");

    // --- STAGE 3: pBIOS MAIN LOOP ---
    while (true) {
        buttonManager->update();
        int encoder_change = encoderManager->getChange();
        Screen* active_screen = stateManager->getActiveScreen();

        if (active_screen) {
            if (encoder_change > 0) active_screen->handleInput({InputEventType::ENCODER_INCREMENT, encoder_change});
            if (encoder_change < 0) active_screen->handleInput({InputEventType::ENCODER_DECREMENT, encoder_change});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_TOP)) active_screen->handleInput({InputEventType::BTN_TOP_PRESS, 1});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_MIDDLE)) active_screen->handleInput({InputEventType::BTN_MIDDLE_PRESS, 1});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_BOTTOM)) active_screen->handleInput({InputEventType::BTN_BOTTOM_PRESS, 1});

            rtcManager->update();
            UIRenderProps props = active_screen->getRenderProps();
            props.top_status_props.date_text = rtcManager->getDateString();
            props.top_status_props.time_text = rtcManager->getTimeString();
            uiManager->render(props);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}