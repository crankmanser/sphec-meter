// File Path: /src/ui/screens/MaintenanceScreen.cpp
// MODIFIED FILE

#include "MaintenanceScreen.h"

MaintenanceScreen::MaintenanceScreen() : _selected_index(0) {
    _menu_items.push_back("Probe Analysis");
    _menu_items.push_back("New Probe");
    _menu_items.push_back("Hardware Self-Test");
    _menu_items.push_back("Live ADC Voltmeter");
    _menu_items.push_back("pBIOS Snapshot");
    _menu_items.push_back("SD Card Formatter");
    _menu_items.push_back("Android Suite");

    _menu_descriptions.push_back("View probe health KPIs.");
    _menu_descriptions.push_back("Reset config for a new probe.");
    _menu_descriptions.push_back("Check status of all components.");
    _menu_descriptions.push_back("View live, raw ADC voltages.");
    _menu_descriptions.push_back("Save diagnostics to SD card.");
    _menu_descriptions.push_back("Format the SD card.");
    _menu_descriptions.push_back("Placeholder for future use.");
}

void MaintenanceScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];
        
        // --- NEW: Add navigation to the Voltmeter screen ---
        if (selected_item == "Live ADC Voltmeter") {
            if (_stateManager) _stateManager->changeState(ScreenState::LIVE_VOLTMETER);
        }
        // STUB: Navigation logic for other items will be added here later.

    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
}

void MaintenanceScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Maintenance";
    
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_bottom_props.line1 = _menu_descriptions[_selected_index];
    }

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Select";
}