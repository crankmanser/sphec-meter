// src/presentation/screens/HomeScreen.cpp
// MODIFIED FILE
#include "presentation/screens/HomeScreen.h"
#include "app/StateManager.h" // For changing state

void HomeScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
        // Example of changing to a menu screen
        // _stateManager->changeState(ScreenState::SCREEN_MAIN_MENU);
    }
}

UIRenderProps HomeScreen::getRenderProps() {
    UIRenderProps props;

    // <<< FIX: breadcrumb_text has been removed from TopStatusProps.
    // This will be handled by the specific OLED props later.
    // props.top_status_props.breadcrumb_text = "Home";

    // <<< FIX: Access button_prompts directly from the master props struct.
    props.button_prompts = {"Measure", "Menu", "Power"};

    // Middle OLED content
    props.oled_middle_props.is_dirty = true;
    
    ProcessedSensorData data_copy;
    if (xSemaphoreTake(g_processed_data_mutex, (TickType_t)10) == pdTRUE) {
        data_copy = g_processed_data;
        xSemaphoreGive(g_processed_data_mutex);
    }

    char ph_str[16];
    snprintf(ph_str, sizeof(ph_str), "pH: %.2f", data_copy.ph_value);
    
    char ec_str[16];
    snprintf(ec_str, sizeof(ec_str), "EC: %.0f uS", data_copy.ec_value);

    props.oled_middle_props.line1 = ph_str;
    props.oled_middle_props.line2 = ec_str;
    
    return props;
}