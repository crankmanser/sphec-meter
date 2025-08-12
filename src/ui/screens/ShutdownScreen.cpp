// File Path: /src/ui/screens/ShutdownScreen.cpp
// MODIFIED FILE

#include "ShutdownScreen.h"
#include "ui/UIManager.h" // Include for UIRenderProps definition
#include "ConfigManager.h"
#include "SdManager.h"

// Make the global managers available to this screen
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
        // --- DEFINITIVE FIX: Implement the shutdown logic for each option ---
        switch (_selected_index) {
            case 0: // Save & Shutdown
                // Save the current working state of all filters to their default files
                configManager.saveFilterSettings(phFilter, "ph_filter", "default", false);
                configManager.saveFilterSettings(ecFilter, "ec_filter", "default", false);
                configManager.saveFilterSettings(v3_3_Filter, "v3_3_filter", "default", false);
                configManager.saveFilterSettings(v5_0_Filter, "v5_0_filter", "default", false);
                break;

            case 1: // Discard & Shutdown
                // Do nothing, any in-memory changes will be lost.
                break;

            case 2: // Restore Defaults & Shutdown
                // Delete the default config files. They will be recreated with
                // hardcoded defaults on the next boot.
                sdManager.remove("/config/ph_filter.json");
                sdManager.remove("/config/ec_filter.json");
                sdManager.remove("/config/v3_3_filter.json");
                sdManager.remove("/config/v5_0_filter.json");
                break;
        }
        // After the action is complete, transition to the final power off screen.
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