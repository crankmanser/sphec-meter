// File Path: /src/ui/screens/pBiosMenuScreen.cpp
// MODIFIED FILE

#include "pBiosMenuScreen.h"
#include "../StateManager.h"
#include <Arduino.h>

pBiosMenuScreen::pBiosMenuScreen() : _selected_index(0) {
    // --- MODIFIED: Update menu items to the new structure ---
    _menu_items.push_back("Noise Analysis");
    _menu_items.push_back("NA Drift Trending");
    _menu_items.push_back("Live Filter Tuning");
    _menu_items.push_back("Maintenance");
    _menu_items.push_back("Shutdown");
    
    // --- MODIFIED: Update descriptions to match ---
    _menu_descriptions.push_back("Analyze HF signal noise (statistical).");
    _menu_descriptions.push_back("Analyze LF signal drift (FFT).");
    _menu_descriptions.push_back("Tune HF/LF filter parameters live.");
    _menu_descriptions.push_back("Run system maintenance tasks.");
    _menu_descriptions.push_back("Prepare device for safe power-off.");
}

void pBiosMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } 
    else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];
        
        if (selected_item == "Live Filter Tuning") {
            if (_stateManager) _stateManager->changeState(ScreenState::FILTER_SELECTION);
        }
        else if (selected_item == "Noise Analysis") {
            if (_stateManager) _stateManager->changeState(ScreenState::NOISE_ANALYSIS);
        }
        else if (selected_item == "NA Drift Trending") {
            if (_stateManager) _stateManager->changeState(ScreenState::DRIFT_TRENDING);
        }
        // --- NEW: Add navigation to new sub-menus ---
        else if (selected_item == "Maintenance") {
            if (_stateManager) _stateManager->changeState(ScreenState::MAINTENANCE_MENU);
        }
        else if (selected_item == "Shutdown") {
            if (_stateManager) _stateManager->changeState(ScreenState::SHUTDOWN_MENU);
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

    props_to_fill->button_props.back_text = "";
    props_to_fill->button_props.enter_text = "";
    props_to_fill->button_props.down_text = "Select";
}