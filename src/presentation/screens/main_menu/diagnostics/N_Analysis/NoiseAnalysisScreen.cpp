// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.cpp
// MODIFIED FILE
#include "NoiseAnalysisScreen.h"
#include "app/StateManager.h"
#include <cmath> // For sin()

NoiseAnalysisScreen::NoiseAnalysisScreen() : _selected_view_index(0) {
    // Generate some sample sine wave data for testing the graph block
    for (int i = 0; i < 128; i++) {
        _sample_data.push_back(sin(i * 0.2f));
    }

    // <<< ADDED: Define the items for the new sub-menu on OLED #2 >>>
    _view_options.push_back("Waveform");
    _view_options.push_back("FFT Analysis");
    _view_options.push_back("Statistics");
}

void NoiseAnalysisScreen::handleInput(const InputEvent& event) {
    // <<< ADDED: Handle input for the new sub-menu >>>
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_view_index < _view_options.size() - 1) {
            _selected_view_index++;
        }
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_view_index > 0) {
            _selected_view_index--;
        }
    }
    
    // Bottom button is always "Back" on this screen
    if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
        if (_stateManager) {
            _stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
        }
    }
}

UIRenderProps NoiseAnalysisScreen::getRenderProps() {
    UIRenderProps props;

    // <<< MODIFIED: Hide the top status bar to maximize screen real estate >>>
    props.show_top_bar = false;

    // --- OLED #1: The Graph Block ---
    // (We will later tie the visibility of this to the menu selection)
    props.oled_top_props.graph_props.is_enabled = true;
    props.oled_top_props.graph_props.title = "Live Noise Waveform";
    props.oled_top_props.graph_props.data_points = &_sample_data;
    props.oled_top_props.graph_props.auto_scale_y = true;

    // --- OLED #2: The Sub-Menu ---
    props.oled_middle_props.menu_props.is_enabled = true;
    props.oled_middle_props.menu_props.items = _view_options;
    props.oled_middle_props.menu_props.selected_index = _selected_view_index;

    // --- OLED #3: Text Status Area ---
    // All text now appears here, to the right of the status icons.
    props.oled_bottom_props.line1 = "PH Probe Analysis";
    props.oled_bottom_props.line2 = "Mean: 1.234V";
    props.oled_bottom_props.line3 = "Std Dev: 0.005V";
    
    // --- Button Prompts ---
    // The top button prompt will appear on OLED #3
    props.button_prompts.top_button_text = "Start/Stop";
    // The bottom button prompt will appear on OLED #1
    props.button_prompts.bottom_button_text = "Back";

    return props;
}