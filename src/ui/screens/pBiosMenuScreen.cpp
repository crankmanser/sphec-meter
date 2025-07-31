// File Path: /src/ui/screens/pBiosMenuScreen.cpp
// NEW FILE

#include "pBiosMenuScreen.h"
#include <Arduino.h> // For ESP.restart()

pBiosMenuScreen::pBiosMenuScreen() : _selected_index(0) {
    _menu_items.push_back("System Info");
    _menu_items.push_back("SD Card Diag");
    _menu_items.push_back("Reboot to Normal");

    _menu_descriptions.push_back("View firmware and hardware status.");
    _menu_descriptions.push_back("Run a diagnostic on the SD card.");
    _menu_descriptions.push_back("Exit diagnostics and restart the device.");
}

void pBiosMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) {
            _selected_index++;
        }
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) {
            _selected_index--;
        }
    } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
        const std::string& selected_item = _menu_items[_selected_index];
        if (selected_item == "Reboot to Normal") {
            ESP.restart();
        }
        // Add logic for other menu items here in the future
    }
}

void pBiosMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    // Top OLED: Contextual Help
    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    // Middle OLED: The Menu
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    // Bottom OLED: Breadcrumbs
    props_to_fill->oled_bottom_props.line1 = "pBios Menu";

    // Button Prompts
    props_to_fill->button_props.enter_text = "Select";
}