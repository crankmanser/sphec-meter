// src/app/UiTask.cpp
// MODIFIED FILE

#include "app/UiTask.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "managers/rtc/RtcManager.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"

// External declarations for global manager pointers
extern UIManager* uiManager;
extern StateManager* stateManager;
extern RtcManager* rtcManager;
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;

// The main loop for the UI will run at approximately 20 FPS
const TickType_t UI_DELAY_MS = 50; 

void uiTask(void* pvParameters) {
    LOG_TASK("UI Task started.\n");

    if (!uiManager || !stateManager || !rtcManager || !buttonManager || !encoderManager) {
        LOG_MAIN("[UI_ERROR] UI Task cannot run, a required manager is null.\n");
        vTaskDelete(NULL);
        return;
    }
    
    randomSeed(esp_random());

    for (;;) {
        // --- STAGE 1: UPDATE INPUT MANAGERS ---
        buttonManager->update();
        int encoder_change = encoderManager->getChange();

        // --- STAGE 2: PROCESS INPUT ---
        Screen* active_screen = stateManager->getActiveScreen();
        if (active_screen) {
            // <<< FIX: Process only ONE step per frame for a smooth scroll >>>
            if (encoder_change > 0) {
                active_screen->handleInput({InputEventType::ENCODER_INCREMENT, 1});
            } else if (encoder_change < 0) {
                // Pass -1 for decrement, even though event.value is not used by menu screen yet
                active_screen->handleInput({InputEventType::ENCODER_DECREMENT, -1});
            }

            if (buttonManager->isPressed(ButtonManager::BTN_TOP)) {
                active_screen->handleInput({InputEventType::BTN_TOP_PRESS, 1});
            }
            if (buttonManager->isPressed(ButtonManager::BTN_MIDDLE)) {
                active_screen->handleInput({InputEventType::BTN_MIDDLE_PRESS, 1});
            }
            if (buttonManager->isPressed(ButtonManager::BTN_BOTTOM)) {
                active_screen->handleInput({InputEventType::BTN_BOTTOM_PRESS, 1});
            }
        }

        // --- STAGE 3: GET RENDER DATA AND DRAW FRAME ---
        rtcManager->update();
        UIRenderProps props = active_screen->getRenderProps();
        props.top_status_props.date_text = rtcManager->getDateString();
        props.top_status_props.time_text = rtcManager->getTimeString();
        uiManager->render(props);

        vTaskDelay(pdMS_TO_TICKS(UI_DELAY_MS));
    }
}