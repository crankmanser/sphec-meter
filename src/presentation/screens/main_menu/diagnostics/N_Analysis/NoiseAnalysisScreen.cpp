// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.cpp
// MODIFIED FILE
#include "NoiseAnalysisScreen.h"
#include "app/StateManager.h"
#include <cmath>

NoiseAnalysisScreen::NoiseAnalysisScreen() : 
    _current_view(ViewState::SELECT_SENSOR),
    _selected_sensor_index(0),
    _selected_view_index(0)
{
    _sensor_menu_items.push_back("pH Probe");
    _sensor_menu_items.push_back("EC Probe");
    _sensor_menu_items.push_back("3.3V Bus");
    _sensor_menu_items.push_back("5.0V Bus");
    _sensor_menu_items.push_back("INA219 Voltage");
    _sensor_menu_items.push_back("INA219 Current");

    for (int i = 0; i < 128; i++) {
        _sample_data.push_back(sin(i * 0.2f));
    }
    
    _view_options.push_back("Waveform");
    _view_options.push_back("FFT Analysis");
    _view_options.push_back("Statistics");
    _view_options.push_back("Live Tuning");
}


void NoiseAnalysisScreen::handleInput(const InputEvent& event) {
    if (_current_view == ViewState::SELECT_SENSOR) {
        if (event.type == InputEventType::ENCODER_INCREMENT) {
            if (_selected_sensor_index < _sensor_menu_items.size() - 1) _selected_sensor_index++;
        } else if (event.type == InputEventType::ENCODER_DECREMENT) {
            if (_selected_sensor_index > 0) _selected_sensor_index--;
        } else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
            _current_view = ViewState::SHOW_ANALYSIS;
        } else if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
            if (_stateManager) _stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
        }
    } else { // _current_view == ViewState::SHOW_ANALYSIS
        if (event.type == InputEventType::ENCODER_INCREMENT) {
            if (_selected_view_index < _view_options.size() - 1) _selected_view_index++;
        } else if (event.type == InputEventType::ENCODER_DECREMENT) {
            if (_selected_view_index > 0) _selected_view_index--;
        } else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
            // <<< This is where the "Run" action will be triggered >>>
            // (Functionality to be added later)
        }
        
        else if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
            _current_view = ViewState::SELECT_SENSOR;
        }
    }
}

UIRenderProps NoiseAnalysisScreen::getRenderProps() {
    UIRenderProps props;
    props.show_top_bar = false;

    if (_current_view == ViewState::SELECT_SENSOR) {
        // --- Props for the Sensor Selection View ---
        props.oled_top_props.line1 = "Please select a signal";
        props.oled_top_props.line2 = "to analyze.";
        
        props.oled_middle_props.menu_props.is_enabled = true;
        props.oled_middle_props.menu_props.items = _sensor_menu_items;
        props.oled_middle_props.menu_props.selected_index = _selected_sensor_index;
        
        props.oled_bottom_props.line1 = "Diag > Noise Analysis";

        // <<< MODIFIED: Only the middle and bottom prompts are used here >>>
        props.button_prompts.middle_button_text = "Select";
        props.button_prompts.bottom_button_text = "Back";

    } else { // _current_view == ViewState::SHOW_ANALYSIS
        // --- Props for the Analysis Results View ---
        props.oled_top_props.graph_props.is_enabled = true;
        props.oled_top_props.graph_props.data_points = &_sample_data;
        props.oled_top_props.graph_props.auto_scale_y = true;
        props.oled_top_props.graph_props.show_top_bar = props.show_top_bar;

        props.oled_middle_props.menu_props.is_enabled = true;
        props.oled_middle_props.menu_props.items = _view_options;
        props.oled_middle_props.menu_props.selected_index = _selected_view_index;

        props.oled_bottom_props.line1 = "Analysis for:";
        props.oled_bottom_props.line2 = _sensor_menu_items[_selected_sensor_index];
        
        // <<< MODIFIED: Only the middle prompt is used for the primary action >>>
        props.button_prompts.middle_button_text = "Run";
        // Top and Bottom prompts are empty for a cleaner look
        props.button_prompts.top_button_text = "";
        props.button_prompts.bottom_button_text = "";
    }

    return props;
}