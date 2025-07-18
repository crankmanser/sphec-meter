// src/app/UiTask.cpp
// MODIFIED FILE
#include "app/UiTask.h"
#include "freertos/task.h"
#include "app/AppContext.h" // <<< ADDED

#include "DebugMacros.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "managers/rtc/RtcManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"

void uiTask(void* pvParameters) {
    LOG_TASK("UI Task started.\n");

    // <<< MODIFIED: Get all dependencies from the AppContext >>>
    AppContext* context = static_cast<AppContext*>(pvParameters);
    StateManager* stateManager = context->stateManager;
    UIManager* uiManager = context->uiManager;
    RtcManager* rtcManager = context->rtcManager;
    ButtonManager* buttonManager = context->buttonManager;
    EncoderManager* encoderManager = context->encoderManager;

    if (!uiManager || !stateManager || !rtcManager || !buttonManager || !encoderManager) {
        LOG_MAIN("[UI_ERROR] UI Task cannot run, a required manager is null in the context.\n");
        vTaskDelete(NULL);
        return;
    }

    // Wait for the StateManager to be ready.
    while (stateManager->getActiveScreen() == nullptr) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    for (;;) {
        // --- STAGE 1: GATHER INPUT ---
        buttonManager->update();
        int encoder_change = encoderManager->getChange();

        // --- STAGE 2: PROCESS INPUT ---
        Screen* active_screen = stateManager->getActiveScreen();
        if (active_screen) {
            if (encoder_change > 0) active_screen->handleInput({InputEventType::ENCODER_INCREMENT, encoder_change});
            if (encoder_change < 0) active_screen->handleInput({InputEventType::ENCODER_DECREMENT, encoder_change});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_TOP)) active_screen->handleInput({InputEventType::BTN_TOP_PRESS, 1});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_MIDDLE)) active_screen->handleInput({InputEventType::BTN_MIDDLE_PRESS, 1});
            if (buttonManager->wasJustPressed(ButtonManager::BTN_BOTTOM)) active_screen->handleInput({InputEventType::BTN_BOTTOM_PRESS, 1});
        }
        
        // --- STAGE 3: GET RENDER DATA & DRAW FRAME ---
        rtcManager->update();
        if (active_screen) {
            UIRenderProps props = active_screen->getRenderProps();
            props.top_status_props.date_text = rtcManager->getDateString();
            props.top_status_props.time_text = rtcManager->getTimeString();
            uiManager->render(props);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}