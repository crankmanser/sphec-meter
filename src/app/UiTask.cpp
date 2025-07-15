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

// External declarations
extern UIManager* uiManager;
extern StateManager* stateManager;
extern RtcManager* rtcManager;
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;

const TickType_t UI_DELAY_MS = 50;

void uiTask(void* pvParameters) {
    LOG_TASK("UI Task started.\n");

    // --- INITIAL SANITY CHECK ---
    // This check ensures that all required managers were successfully instantiated
    // before the task scheduler started. If any are null, the UI cannot function.
    if (!uiManager || !stateManager || !rtcManager || !buttonManager || !encoderManager) {
        LOG_MAIN("[UI_ERROR] UI Task cannot run, a required manager is null.\n");
        vTaskDelete(NULL);
        return;
    }

    randomSeed(esp_random());

    for (;;) {
        // --- STAGE 1: WAIT FOR SYSTEM READY ---
        // This is the critical fix. The StateManager might not have an active screen
        // immediately at boot. This check prevents the task from proceeding until
        // the StateManager is fully configured, preventing a null pointer crash.
        Screen* active_screen = stateManager->getActiveScreen();
        if (active_screen == nullptr) {
            vTaskDelay(pdMS_TO_TICKS(UI_DELAY_MS));
            continue; // Skip the rest of the loop and try again
        }

        // --- STAGE 2: UPDATE INPUT MANAGERS ---
        // Poll the hardware input managers to get the latest state of the buttons
        // and the accumulated change from the rotary encoder task.
        buttonManager->update();
        int encoder_change = encoderManager->getChange();

        // --- STAGE 3: PROCESS INPUT ---
        // Pass the collected input events to the currently active screen for handling.
        // The screen itself contains the logic for what to do with these inputs.
        if (encoder_change > 0) {
            active_screen->handleInput({InputEventType::ENCODER_INCREMENT, encoder_change});
        } else if (encoder_change < 0) {
            active_screen->handleInput({InputEventType::ENCODER_DECREMENT, encoder_change});
        }

        if (buttonManager->wasJustPressed(ButtonManager::BTN_TOP)) {
            active_screen->handleInput({InputEventType::BTN_TOP_PRESS, 1});
        }
        if (buttonManager->wasJustPressed(ButtonManager::BTN_MIDDLE)) {
            active_screen->handleInput({InputEventType::BTN_MIDDLE_PRESS, 1});
        }
        if (buttonManager->wasJustPressed(ButtonManager::BTN_BOTTOM)) {
            active_screen->handleInput({InputEventType::BTN_BOTTOM_PRESS, 1});
        }

        // --- STAGE 4: GET RENDER DATA AND DRAW FRAME ---
        // Ask the active screen for its current render properties. Then, update
        // the real-time data (like the clock) and pass the final properties
        // to the UIManager to render the complete frame.
        rtcManager->update();
        UIRenderProps props = active_screen->getRenderProps();
        props.top_status_props.date_text = rtcManager->getDateString();
        props.top_status_props.time_text = rtcManager->getTimeString();
        uiManager->render(props);

        // --- STAGE 5: DELAY ---
        // Wait for a short period before the next loop iteration to yield
        // CPU time to other tasks.
        vTaskDelay(pdMS_TO_TICKS(UI_DELAY_MS));
    }
}