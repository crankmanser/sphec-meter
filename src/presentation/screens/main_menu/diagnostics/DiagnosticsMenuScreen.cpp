// src/presentation/screens/main_menu/diagnostics/DiagnosticsMenuScreen.cpp
// MODIFIED FILE
#include "DiagnosticsMenuScreen.h"
#include "app/StateManager.h"

DiagnosticsMenuScreen::DiagnosticsMenuScreen() : _selected_index(0) {
    _menu_items.push_back("Noise Analysis");
    // _menu_items.push_back("--- Back ---"); // No longer needed

    _menu_descriptions.push_back("Measure raw ADC signal noise.");
    // _menu_descriptions.push_back("Return to the main menu."); // No longer needed
}

void DiagnosticsMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) {
            _selected_index++;
        }
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) {
            _selected_index--;
        }
    } else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
        // <<< MODIFIED: Navigate to the new noise analysis screen >>>
        if (_menu_items[_selected_index] == "Noise Analysis") {
            if (_stateManager) {
                _stateManager->changeState(ScreenState::SCREEN_NOISE_ANALYSIS);
            }
        }
    } else if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
        if (_stateManager) {
            _stateManager->changeState(ScreenState::SCREEN_MAIN_MENU);
        }
    }
}

UIRenderProps DiagnosticsMenuScreen::getRenderProps() {
    UIRenderProps props;

    // OLED #1: Contextual Description
    if (!_menu_descriptions.empty() && _selected_index < _menu_descriptions.size()) {
        props.oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    // OLED #2: The Menu
    props.oled_middle_props.menu_props.is_enabled = true;
    props.oled_middle_props.menu_props.items = _menu_items;
    props.oled_middle_props.menu_props.selected_index = _selected_index;

    // OLED #3: Breadcrumbs
    props.oled_bottom_props.line1 = "Home >";
    props.oled_bottom_props.line2 = "Diagnostics";
    
    // Button Prompts
    props.button_prompts.middle_button_text = "Run";
    // <<< MODIFIED: Bottom button is now the back button >>>
    props.button_prompts.bottom_button_text = "Back";

    return props;
}