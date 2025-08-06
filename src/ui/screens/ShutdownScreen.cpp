// File Path: /src/ui/screens/ShutdownScreen.cpp
// NEW FILE

#include "ShutdownScreen.h"

ShutdownScreen::ShutdownScreen() : _selected_index(0) {
    _menu_items.push_back("Save & Shutdown");
    _menu_items.push_back("Discard & Shutdown");
    _menu_items.push_back("Restore Defaults & Shutdown");
}

void ShutdownScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        // STUB: Logic for each shutdown option will be added here.
        // This will eventually lead to a "Safe to Power Off" message screen.
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
}

void ShutdownScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Shutdown";
    
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.line1 = "Confirm shutdown option.";

    props_to_fill->button_props.back_text = "Cancel";
    props_to_fill->button_props.down_text = "Confirm";
}