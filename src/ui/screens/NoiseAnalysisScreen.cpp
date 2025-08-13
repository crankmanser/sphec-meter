// File Path: /src/ui/screens/NoiseAnalysisScreen.cpp
// MODIFIED FILE

#include "NoiseAnalysisScreen.h"
#include "pBiosContext.h"
#include "AdcManager.h"
#include "ADS1118.h"
#include <stdio.h>
#include "ui/UIManager.h" // Include for UIRenderProps definition

/**
 * @brief --- DEFINITIVE REFACTOR: Constructor signature updated ---
 * The AdcManager is no longer passed in, as this screen is no longer
 * responsible for managing the probe's power state.
 */
NoiseAnalysisScreen::NoiseAnalysisScreen(PBiosContext* context) :
    _context(context),
    _current_state(AnalysisState::SELECT_SOURCE),
    _selected_source_index(0),
    _sampling_progress_percent(0),
    _result_mean(0.0),
    _result_min(0.0),
    _result_max(0.0),
    _result_pk_pk(0.0),
    _result_std_dev(0.0)
{
    _source_menu_items.push_back("pH Probe");
    _source_menu_items.push_back("EC Probe");
    _source_menu_items.push_back("3.3V Bus");
    _source_menu_items.push_back("5.0V Bus");
    for(int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
        _result_samples[i] = 0.0;
    }
}

void NoiseAnalysisScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _current_state = AnalysisState::SELECT_SOURCE;
    _sampling_progress_percent = 0;
}

void NoiseAnalysisScreen::handleInput(const InputEvent& event) {
    switch (_current_state) {
        case AnalysisState::SELECT_SOURCE: handleSelectSourceInput(event); break;
        case AnalysisState::SAMPLING: break;
        case AnalysisState::VIEW_RESULTS: handleViewResultsInput(event); break;
    }
}

void NoiseAnalysisScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    switch (_current_state) {
        case AnalysisState::SELECT_SOURCE: getSelectSourceRenderProps(props_to_fill); break;
        case AnalysisState::SAMPLING: getSamplingRenderProps(props_to_fill); break;
        case AnalysisState::VIEW_RESULTS: getViewResultsRenderProps(props_to_fill); break;
    }
}

void NoiseAnalysisScreen::setAnalysisResults(double mean, double min, double max, double pk_pk, double std_dev, const std::vector<double>& samples) {
    _result_mean = mean;
    _result_min = min;
    _result_max = max;
    _result_pk_pk = pk_pk;
    _result_std_dev = std_dev;
    for (int i = 0; i < ANALYSIS_SAMPLE_COUNT; ++i) {
        if (i < samples.size()) {
            _result_samples[i] = samples[i];
        }
    }
    _current_state = AnalysisState::VIEW_RESULTS;
}

void NoiseAnalysisScreen::setSamplingProgress(int percent) {
    _sampling_progress_percent = percent;
}

bool NoiseAnalysisScreen::isSampling() const {
    return _current_state == AnalysisState::SAMPLING;
}

/**
 * @brief --- DEFINITIVE REFACTOR: Input handler no longer controls hardware ---
 * This method now only sets the shared context and changes the UI state.
 * The controller in main.cpp will see the state change and activate the probe.
 */
void NoiseAnalysisScreen::handleSelectSourceInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_source_index < _source_menu_items.size() - 1) _selected_source_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_source_index > 0) _selected_source_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_context) {
            switch (_selected_source_index) {
                case 0: _context->selectedAdcIndex = 0; _context->selectedAdcInput = ADS1118::DIFF_0_1; break;
                case 1: _context->selectedAdcIndex = 1; _context->selectedAdcInput = ADS1118::DIFF_0_1; break;
                case 2: _context->selectedAdcIndex = 0; _context->selectedAdcInput = ADS1118::AIN_2; break;
                case 3: _context->selectedAdcIndex = 1; _context->selectedAdcInput = ADS1118::AIN_2; break;
            }
            _sampling_progress_percent = 0;
            _current_state = AnalysisState::SAMPLING;
        }
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
}

/**
 * @brief --- DEFINITIVE REFACTOR: Input handler no longer controls hardware ---
 * Deactivation is handled automatically when the state changes.
 */
void NoiseAnalysisScreen::handleViewResultsInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_DOWN_PRESS) {
        _current_state = AnalysisState::SELECT_SOURCE;
    }
}

void NoiseAnalysisScreen::getSelectSourceRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Noise Analysis";
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _source_menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_source_index;
    props_to_fill->oled_bottom_props.line1 = "Select source to analyze.";
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "";
    props_to_fill->button_props.down_text = "Analyze";
}

void NoiseAnalysisScreen::getSamplingRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Noise Analysis";
    props_to_fill->oled_bottom_props.line1 = "Acquiring high-speed data...";
    props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
    props_to_fill->oled_middle_props.progress_bar_props.label = "Sampling...";
    props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _sampling_progress_percent;
}

void NoiseAnalysisScreen::getViewResultsRenderProps(UIRenderProps* props_to_fill) {
    char buffer[32];

    props_to_fill->oled_top_props.line1 = "Results: " + _source_menu_items[_selected_source_index];
    snprintf(buffer, sizeof(buffer), "Mean:   %.2f mV", _result_mean);
    props_to_fill->oled_top_props.line2 = buffer;
    snprintf(buffer, sizeof(buffer), "StdDev: %.2f mV", _result_std_dev);
    props_to_fill->oled_top_props.line3 = buffer;

    snprintf(buffer, sizeof(buffer), "Min:   %.2f mV", _result_min);
    props_to_fill->oled_middle_props.line1 = buffer;
    snprintf(buffer, sizeof(buffer), "Max:   %.2f mV", _result_max);
    props_to_fill->oled_middle_props.line2 = buffer;
    snprintf(buffer, sizeof(buffer), "Pk-Pk: %.2f mV", _result_pk_pk);
    props_to_fill->oled_middle_props.line3 = buffer;

    props_to_fill->oled_bottom_props.line1 = "Press any button to continue.";

    props_to_fill->button_props.back_text = "Done";
    props_to_fill->button_props.enter_text = "";
    props_to_fill->button_props.down_text = "Done";
}