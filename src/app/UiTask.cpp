// src/app/UiTask.cpp
// MODIFIED FILE

#include "app/UiTask.h"
#include "freertos/task.h"

#include "DebugMacros.h"
#include "presentation/UIManager.h"
#include "app/StateManager.h"
#include "presentation/resources/icons.h"
#include "managers/rtc/RtcManager.h"

extern UIManager* uiManager;
extern StateManager* stateManager;
extern RtcManager* rtcManager;

const TickType_t UI_DELAY_MS = 500; // Slowed down to 2 FPS for easy visual checking

void uiTask(void* pvParameters) {
    LOG_TASK("UI Task started.\n");

    if (!stateManager || !uiManager || !rtcManager) {
        LOG_MAIN("[UI_ERROR] UI Task cannot run, manager pointers are null.\n");
        vTaskDelete(NULL);
        return;
    }
    // Initialize random seed
    randomSeed(analogRead(0));

    for (;;) {
        // Update managers that provide data to the UI
        rtcManager->update();
        UIRenderProps props = stateManager->getActiveScreen()->getRenderProps();

        // --- VISUAL VALIDATION: Override props with random icons ---
        props.top_status_props.ph_probe_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.ph_kpi_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.ec_probe_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.ec_kpi_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.sd_card_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.wifi_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.bus_3v3_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.bus_5v_icon = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.top_status_props.date_text = rtcManager->getDateString();
        props.top_status_props.time_text = rtcManager->getTimeString();

        props.state_stack_props.icon1 = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.state_stack_props.icon2 = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));
        props.state_stack_props.icon3 = static_cast<Icon_ID>(random((int)Icon_ID::ICON_COUNT));

        uiManager->render(props);

        vTaskDelay(pdMS_TO_TICKS(UI_DELAY_MS));
    }
}