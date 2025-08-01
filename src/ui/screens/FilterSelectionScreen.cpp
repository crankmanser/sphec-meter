// File Path: /src/ui/screens/FilterSelectionScreen.cpp
// MODIFIED FILE

#include "FilterSelectionScreen.h"
#include "pBiosContext.h" 
#include "ADS1118.h"       


FilterSelectionScreen::FilterSelectionScreen(PBiosContext* context) : 
    _context(context),
    _selected_index(0) 
{
    _menu_items.push_back("pH Probe Filter");
    _menu_items.push_back("EC Probe Filter");
    _menu_items.push_back("3.3V Bus Filter");
    _menu_items.push_back("5.0V Bus Filter");
}

void FilterSelectionScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } 
    // --- FIX: The "Select" action is now triggered by the BOTTOM button ---
    else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_stateManager && _context) {
            // ... (context setting logic is unchanged) ...
            _stateManager->changeState(ScreenState::LIVE_FILTER_TUNING);
        }
    } 
    // The top button is now consistently "Back"
    else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
}

void FilterSelectionScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Live Tuning";
    
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.line1 = "Select filter to tune.";

    // --- Button Prompts Updated ---
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "";
    props_to_fill->button_props.down_text = "Select";
}