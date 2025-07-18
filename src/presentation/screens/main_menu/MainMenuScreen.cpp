// src/presentation/screens/main_menu/MainMenuScreen.cpp
// MODIFIED FILE
#include "MainMenuScreen.h"
#include "app/StateManager.h"
#include "app/common/system_control.h"
#include "DebugMacros.h"

MainMenuScreen::MainMenuScreen() : _selected_index(0) {
    _menu_items.push_back("Measure");
    _menu_items.push_back("Diagnostics");
    _menu_items.push_back("Settings");
    _menu_items.push_back("Shutdown");

    _menu_descriptions.push_back("View live sensor readings.");
    _menu_descriptions.push_back("Run system health checks.");
    _menu_descriptions.push_back("Configure device options.");
    _menu_descriptions.push_back("Safely power down the device.");
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
    }
    else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];

        if (selected_item == "Diagnostics") {
            if (_stateManager) {
                _stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
            }
        }
        else if (selected_item == "Shutdown") {
            // <<< MODIFIED: Pass the context to the shutdown function >>>
            initiate_shutdown(_appContext);
        }
    }
}

UIRenderProps MainMenuScreen::getRenderProps() {
    UIRenderProps props;

    if (_selected_index < _menu_descriptions.size()) {
        props.oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    props.oled_middle_props.menu_props.is_enabled = true;
    props.oled_middle_props.menu_props.items = _menu_items;
    props.oled_middle_props.menu_props.selected_index = _selected_index;

    props.oled_bottom_props.line1 = "Home";
    props.button_prompts.middle_button_text = "Select";

    return props;
}