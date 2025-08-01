// File Path: /src/ui/screens/pBiosMenuScreen.cpp
// MODIFIED FILE

#include "pBiosMenuScreen.h"
#include "../StateManager.h"
#include <Arduino.h>

pBiosMenuScreen::pBiosMenuScreen() : _selected_index(0) {
    _menu_items.push_back("Noise Analysis");
    _menu_items.push_back("NA Drift Trending");
    _menu_items.push_back("Live Filter Tuning");
    _menu_items.push_back("Performance Index");
    _menu_items.push_back("Maintenance");
    // "Reboot to Normal" is no longer needed with the new boot system
    
    _menu_descriptions.push_back("Analyze HF and LF signal noise.");
    _menu_descriptions.push_back("View historical noise level trends.");
    _menu_descriptions.push_back("Tune HF/LF filter parameters live.");
    _menu_descriptions.push_back("View calibration and probe KPIs.");
    _menu_descriptions.push_back("Run system maintenance tasks.");
}

void pBiosMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } 
    // --- FIX: The "Select" action is now triggered by the BOTTOM button ---
    else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];
        
        if (selected_item == "Live Filter Tuning") {
            if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION);
        }
    }
}

void pBiosMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios Home";
    
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_bottom_props.line1 = _menu_descriptions[_selected_index];
    }

    // --- Button Prompts Updated ---
    props_to_fill->button_props.back_text = ""; // No back action
    props_to_fill->button_props.enter_text = ""; // Middle button unused
    props_to_fill->button_props.down_text = "Select"; // Bottom button is Select
}