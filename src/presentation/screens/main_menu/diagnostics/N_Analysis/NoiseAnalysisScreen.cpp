// src/presentation/screens/main_menu/diagnostics/N_Analysis/NoiseAnalysisScreen.cpp
// MODIFIED FILE
#include "NoiseAnalysisScreen.h"
#include "app/StateManager.h"
#include "app/NoiseAnalysisTask.h" // <<< ADDED
#include "DebugMacros.h"          // <<< ADDED
#include <cmath>


NoiseAnalysisScreen::NoiseAnalysisScreen(NoiseAnalysisManager* noiseAnalysisManager) :
    _noise_analysis_manager(noiseAnalysisManager),
    _current_view(ViewState::SELECT_SENSOR),
    _selected_sensor_index(0),
    _selected_view_index(0),
    _analysis_task_handle(NULL),
    _analysis_complete_flag(false),
    _analysis_start_time_ms(0),
    _estimated_duration_ms(2000) // Estimate 2 seconds for the analysis
{
    _sensor_menu_items.push_back("pH Probe");
    _sensor_menu_items.push_back("EC Probe");
    _sensor_menu_items.push_back("3.3V Bus");
    _sensor_menu_items.push_back("5.0V Bus");
    _sensor_menu_items.push_back("INA219 Voltage");
    _sensor_menu_items.push_back("INA219 Current");

    _view_options.push_back("Statistics");
    _view_options.push_back("FFT Analysis");
    _view_options.push_back("Recommended Tuning");
}

void NoiseAnalysisScreen::onEnter(StateManager* stateManager) {
    _stateManager = stateManager;
    // Reset the screen to its initial state every time we enter it
    _current_view = ViewState::SELECT_SENSOR;
    _analysis_complete_flag = false;
    LOG_UI("Entered NoiseAnalysisScreen.\n");
}

void NoiseAnalysisScreen::handleInput(const InputEvent& event) {
    switch (_current_view) {
        case ViewState::SELECT_SENSOR:
            if (event.type == InputEventType::ENCODER_INCREMENT) {
                if (_selected_sensor_index < _sensor_menu_items.size() - 1) _selected_sensor_index++;
            } else if (event.type == InputEventType::ENCODER_DECREMENT) {
                if (_selected_sensor_index > 0) _selected_sensor_index--;
            } else if (event.type == InputEventType::BTN_MIDDLE_PRESS) {
                // --- This is where we start the analysis ---
                if (_noise_analysis_manager) {
                    LOG_UI("Starting noise analysis for sensor %d.\n", _selected_sensor_index);
                    _analysis_complete_flag = false;
                    _analysis_start_time_ms = millis();
                    _current_view = ViewState::ANALYZING;

                    // Configure and launch the one-shot analysis task
                    _analysis_params = { _noise_analysis_manager, static_cast<SensorToAnalyze>(_selected_sensor_index), &_analysis_complete_flag };
                    xTaskCreatePinnedToCore(
                        noiseAnalysisTask,
                        "NoiseAnalysisTask",
                        4096,
                        &_analysis_params,
                        5, // High priority
                        &_analysis_task_handle,
                        0  // Pin to Core 0
                    );
                }
            } else if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
                if (_stateManager) _stateManager->changeState(ScreenState::SCREEN_DIAGNOSTICS_MENU);
            }
            break;

        case ViewState::ANALYZING:
            // Intentionally block input while analysis is running, except for cancel.
            if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
                 LOG_UI("User cancelled noise analysis.\n");
                 if(_analysis_task_handle != NULL) {
                    vTaskDelete(_analysis_task_handle);
                    _analysis_task_handle = NULL;
                 }
                 _current_view = ViewState::SELECT_SENSOR;
            }
            break;

        case ViewState::SHOW_ANALYSIS:
            if (event.type == InputEventType::ENCODER_INCREMENT) {
                if (_selected_view_index < _view_options.size() - 1) _selected_view_index++;
            } else if (event.type == InputEventType::ENCODER_DECREMENT) {
                if (_selected_view_index > 0) _selected_view_index--;
            } else if (event.type == InputEventType::BTN_BOTTOM_PRESS) {
                _current_view = ViewState::SELECT_SENSOR;
            }
            break;
    }
}

UIRenderProps NoiseAnalysisScreen::getRenderProps() {
    UIRenderProps props;
    props.show_top_bar = false;

    // Check if a completed analysis is ready
    if (_analysis_complete_flag) {
        LOG_UI("Analysis complete, fetching results.\n");
        _stat_results = _noise_analysis_manager->getStatisticalResult();
        _fft_results = _noise_analysis_manager->getFftResult();
        _tuning_results = _noise_analysis_manager->getRecommendedTuning();
        _current_view = ViewState::SHOW_ANALYSIS;
        _analysis_complete_flag = false; // Reset flag
    }

    switch (_current_view) {
        case ViewState::SELECT_SENSOR:
            props.oled_top_props.line1 = "Please select a signal";
            props.oled_top_props.line2 = "to analyze.";
            props.oled_middle_props.menu_props.is_enabled = true;
            props.oled_middle_props.menu_props.items = _sensor_menu_items;
            props.oled_middle_props.menu_props.selected_index = _selected_sensor_index;
            props.oled_bottom_props.line1 = "Diag > Noise Analysis";
            props.button_prompts.middle_button_text = "Select";
            props.button_prompts.bottom_button_text = "Back";
            break;

        case ViewState::ANALYZING:
            {
                props.oled_middle_props.progress_bar_props.is_enabled = true;
                props.oled_middle_props.progress_bar_props.title = "Analyzing...";
                props.button_prompts.bottom_button_text = "Cancel";

                uint32_t elapsed_ms = millis() - _analysis_start_time_ms;
                int progress = 0;

                // --- "Dumb but Intelligent" Progress Bar Logic ---
                uint32_t ninety_five_percent_time = _estimated_duration_ms * 0.95;
                if (elapsed_ms < ninety_five_percent_time) {
                    progress = (elapsed_ms * 95) / ninety_five_percent_time;
                } else {
                    progress = 95;
                    uint32_t remaining_time = _estimated_duration_ms - ninety_five_percent_time;
                    uint32_t stage2_time = remaining_time / 2; // 95-97%
                    uint32_t stage3_time = remaining_time / 2; // 97-99%

                    if (elapsed_ms < ninety_five_percent_time + stage2_time) {
                        progress = 95 + ((elapsed_ms - ninety_five_percent_time) * 2) / stage2_time;
                    } else if (elapsed_ms < ninety_five_percent_time + stage2_time + stage3_time) {
                        progress = 97 + ((elapsed_ms - ninety_five_percent_time - stage2_time) * 2) / stage3_time;
                    } else {
                        progress = 99;
                    }
                }
                props.oled_middle_props.progress_bar_props.progress_percent = progress;
            }
            break;

        case ViewState::SHOW_ANALYSIS:
            props.oled_bottom_props.line1 = "Analysis for:";
            props.oled_bottom_props.line2 = _sensor_menu_items[_selected_sensor_index];
            props.button_prompts.bottom_button_text = "Back";

            const std::string& selected_view = _view_options[_selected_view_index];
            props.oled_top_props.line1 = selected_view;

            if (selected_view == "Statistics" && _stat_results.is_valid) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "Mean: %.4f", _stat_results.mean);
                props.oled_middle_props.line1 = buffer;
                snprintf(buffer, sizeof(buffer), "StdDev: %.6f", _stat_results.std_dev);
                props.oled_middle_props.line2 = buffer;
            } else if (selected_view == "FFT Analysis" && _fft_results.is_valid) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "Dom Freq: %.2f Hz", _fft_results.dominant_frequency);
                props.oled_middle_props.line1 = buffer;
            } else if (selected_view == "Recommended Tuning") {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "Settle: %.4f", _tuning_results.settle_threshold);
                props.oled_middle_props.line1 = buffer;
                snprintf(buffer, sizeof(buffer), "Track R: %.2f", _tuning_results.track_response);
                props.oled_middle_props.line2 = buffer;
            }
            props.oled_middle_props.menu_props.is_enabled = true;
            props.oled_middle_props.menu_props.items = _view_options;
            props.oled_middle_props.menu_props.selected_index = _selected_view_index;
            break;
    }
    return props;
}