// src/presentation/screens/main_menu/MainMenuScreen.cpp
// MODIFIED FILE
#include "MainMenuScreen.h"
#include "app/StateManager.h"

MainMenuScreen::MainMenuScreen() : _selected_index(0) {
    _menu_items.push_back("Measure");
    _menu_items.push_back("Diagnostics");
    _menu_items.push_back("Settings");

    _menu_descriptions.push_back("View live sensor readings.");
    _menu_descriptions.push_back("Run system health checks.");
    _menu_descriptions.push_back("Configure device options.");
}

void MainMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) {
            _selected_index++;
        }
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) {
            _selected_index--;
        }
    } else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
        if (_menu_items[_selected_index] == "Diagnostics") {
            if (_stateManager) {
                _stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
            }
        }
    }
}

UIRenderProps MainMenuScreen::getRenderProps() {
    UIRenderProps props;

    // OLED #1: Contextual Description
    if (_selected_index < _menu_descriptions.size()) {
        props.oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    // OLED #2: The Menu
    props.oled_middle_props.menu_props.is_enabled = true;
    props.oled_middle_props.menu_props.items = _menu_items;
    props.oled_middle_props.menu_props.selected_index = _selected_index;

    // OLED #3: Breadcrumbs
    props.oled_bottom_props.line1 = "Home";
    
    // --- Button Prompts ---
    // <<< MODIFIED: "Select" is now correctly tied to the middle button prompt >>>
    props.button_prompts.middle_button_text = "Select";

    return props;
}