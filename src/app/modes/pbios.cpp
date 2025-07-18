// src/app/modes/pbios.cpp
// MODIFIED FILE
#include "pbios.h"
#include "app/AppContext.h" // <<< ADDED
#include "presentation/DisplayManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "app/StateManager.h"
#include "presentation/UIManager.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include "managers/rtc/RtcManager.h" 
#include "presentation/screens/main_menu/diagnostics/DiagnosticsMenuScreen.h" 
#include "presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.h" 
#include "DebugMacros.h"

void run_pbios_mode(AppContext* appContext) {
    LOG_MAIN("--- Entering pBios (DIAGNOSTICS) Mode ---\n");

    // Get managers from context
    ButtonManager* buttonManager = appContext->buttonManager;
    EncoderManager* encoderManager = appContext->encoderManager;
    StateManager* stateManager = appContext->stateManager;
    UIManager* uiManager = appContext->uiManager;
    RtcManager* rtcManager = appContext->rtcManager;
    NoiseAnalysisManager* noiseAnalysisManager = appContext->noiseAnalysisManager;

    uiManager->begin();

    // Setup screens
    stateManager->addScreen(ScreenState::SCREEN_DIAGNOSTICS_MENU, new DiagnosticsMenuScreen());
    stateManager->addScreen(ScreenState::SCREEN_NOISE_ANALYSIS, new NoiseAnalysisScreen(noiseAnalysisManager));
    
    // <<< MODIFIED: Pass the context to the StateManager's begin method >>>
    stateManager->begin(appContext);

    // The UiTask for pBios mode.
    xTaskCreatePinnedToCore(
        [](void* params) {
            AppContext* context = static_cast<AppContext*>(params);
            ButtonManager* bm = context->buttonManager;
            EncoderManager* em = context->encoderManager;
            StateManager* sm = context->stateManager;
            UIManager* um = context->uiManager;
            RtcManager* rm = context->rtcManager;

            while (true) {
                bm->update();
                int encoder_change = em->getChange();
                Screen* active_screen = sm->getActiveScreen();

                if (active_screen) {
                    if (encoder_change > 0) active_screen->handleInput({InputEventType::ENCODER_INCREMENT, encoder_change});
                    if (encoder_change < 0) active_screen->handleInput({InputEventType::ENCODER_DECREMENT, encoder_change});
                    if (bm->wasJustPressed(ButtonManager::BTN_TOP)) active_screen->handleInput({InputEventType::BTN_TOP_PRESS, 1});
                    if (bm->wasJustPressed(ButtonManager::BTN_MIDDLE)) active_screen->handleInput({InputEventType::BTN_MIDDLE_PRESS, 1});
                    if (bm->wasJustPressed(ButtonManager::BTN_BOTTOM)) active_screen->handleInput({InputEventType::BTN_BOTTOM_PRESS, 1});

                    rm->update();
                    UIRenderProps props = active_screen->getRenderProps();
                    props.top_status_props.date_text = rm->getDateString();
                    props.top_status_props.time_text = rm->getTimeString();
                    um->render(props);
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        },
        "pBiosUiTask", 4096, appContext, 3, NULL, 1);


    LOG_MAIN("pBios initialization complete. UI is now managed by its own task.\n");
    
    // The main pBios thread can now end.
    vTaskDelete(NULL);
}