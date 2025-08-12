// File Path: /src/ui/screens/MainMenuScreen.cpp
// MODIFIED FILE

#include "MainMenuScreen.h"
#include "ui/UIManager.h" // Include for UIRenderProps definition

MainMenuScreen::MainMenuScreen() : _selected_index(0) {
    _menu_items.push_back("Measure");
    _menu_items.push_back("Calibrate");
    _menu_items.push_back("Settings");
    _menu_items.push_back("Shutdown");

    _menu_descriptions.push_back("View live sensor readings.");
    _menu_descriptions.push_back("Calibrate pH and EC probes.");
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
    } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
        // --- DEFINITIVE FIX: Navigate to the correct sub-menu ---
        if (_stateManager) {
            const std::string& selected_item = _menu_items[_selected_index];
            if (selected_item == "Measure") {
                _stateManager->changeState(ScreenState::MEASURE_MENU);
            }
            // STUB: Add navigation for other main app screens here.
        }
    }
}

void MainMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // --- Top OLED: Contextual Help ---
    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    // --- Middle OLED: The Menu ---
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    // --- Bottom OLED: Breadcrumbs ---
    props_to_fill->oled_bottom_props.line1 = "Main Menu";

    // --- Button Prompts ---
    props_to_fill->button_props.enter_text = "Select";
}