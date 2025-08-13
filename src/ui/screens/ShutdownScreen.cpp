// File Path: /src/ui/screens/ShutdownScreen.cpp
// MODIFIED FILE

#include "ShutdownScreen.h"
#include "ui/UIManager.h"
#include "ConfigManager.h"
#include "SdManager.h"

extern ConfigManager configManager;
extern SdManager sdManager;
extern FilterManager phFilter, ecFilter, v3_3_Filter, v5_0_Filter;
extern char g_sessionTimestamp[20];


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
        switch (_selected_index) {
            case 0: // Save & Shutdown
                // --- DEFINITIVE FIX: Save the current working state to the primary operational files. ---
                configManager.saveFilterSettings(phFilter, "ph_filter", "default");
                configManager.saveFilterSettings(ecFilter, "ec_filter", "default");
                configManager.saveFilterSettings(v3_3_Filter, "v3_3_filter", "default");
                configManager.saveFilterSettings(v5_0_Filter, "v5_0_filter", "default");
                break;

            case 1: // Discard & Shutdown
                break;

            case 2: // Restore Defaults & Shutdown
                sdManager.remove("/config/ph_filter.json");
                sdManager.remove("/config/ec_filter.json");
                sdManager.remove("/config/v3_3_filter.json");
                sdManager.remove("/config/v5_0_filter.json");
                break;
        }
        if (_stateManager) _stateManager->changeState(ScreenState::POWER_OFF);

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