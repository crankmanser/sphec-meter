// File Path: /src/ui/screens/ProbeProfilingScreen.cpp
// MODIFIED FILE

#include "ProbeProfilingScreen.h"
#include "ADS1118.h"
#include <stdio.h>
#include "ui/UIManager.h" // Include for UIRenderProps definition

ProbeProfilingScreen::ProbeProfilingScreen() :
    _current_state(ProfilingState::SELECT_PROBE),
    _selected_index(0),
    _live_r_std(0.0),
    _zero_point_drift(0.0),
    _cal_quality_score(0.0)
{
    _menu_items.push_back("pH Probe");
    _menu_items.push_back("EC Probe");
}

void ProbeProfilingScreen::onEnter(StateManager* stateManager) {
    Screen::onEnter(stateManager);
    _current_state = ProfilingState::SELECT_PROBE;
}

void ProbeProfilingScreen::handleInput(const InputEvent& event) {
    switch (_current_state) {
        case ProfilingState::SELECT_PROBE:
            handleSelectProbeInput(event);
            break;
        case ProfilingState::VIEW_REPORT:
            handleViewReportInput(event);
            break;
        case ProfilingState::ANALYZING:
            // Input is ignored while analyzing
            break;
    }
}

/**
 * @brief --- MODIFIED: Renders the full, three-OLED "Report Card". ---
 * This function now displays the complete set of live and historical KPIs
 * according to our finalized design.
 */
void ProbeProfilingScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps(); // Clear previous props

    switch (_current_state) {
        case ProfilingState::SELECT_PROBE:
            props_to_fill->oled_top_props.line1 = "pBios > Probe Profiling";
            props_to_fill->oled_middle_props.menu_props.is_enabled = true;
            props_to_fill->oled_middle_props.menu_props.items = _menu_items;
            props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;
            props_to_fill->oled_bottom_props.line1 = "Select probe to profile.";
            props_to_fill->button_props.back_text = "Back";
            props_to_fill->button_props.down_text = "Analyze";
            break;

        case ProfilingState::ANALYZING:
            props_to_fill->oled_top_props.line1 = "Probe Profiling";
            props_to_fill->oled_middle_props.line1 = "Analyzing signal...";
            props_to_fill->oled_middle_props.line2 = "Please wait.";
            break;

        case ProfilingState::VIEW_REPORT:
            char buffer[40];

            // --- Top OLED: Primary Health Indicators ---
            props_to_fill->oled_top_props.line1 = "Profile: " + _menu_items[_selected_index];
            snprintf(buffer, sizeof(buffer), "ZP Drift: %.3f mV", _zero_point_drift);
            props_to_fill->oled_top_props.line2 = buffer;
            snprintf(buffer, sizeof(buffer), "Live R_std: %.3f mV", _live_r_std);
            props_to_fill->oled_top_props.line3 = buffer;

            // --- Middle OLED: Filter Load (Creep) ---
            props_to_fill->oled_middle_props.line1 = "--- Filter Load ---";
            snprintf(buffer, sizeof(buffer), "HF Settle: %.2f | LF: %.2f", _hf_params_snapshot.settleThreshold, _lf_params_snapshot.settleThreshold);
            props_to_fill->oled_middle_props.line2 = buffer;
            snprintf(buffer, sizeof(buffer), "HF Smooth: %.2f | LF: %.3f", _hf_params_snapshot.lockSmoothing, _lf_params_snapshot.lockSmoothing);
            props_to_fill->oled_middle_props.line3 = buffer;
            
            // --- Bottom OLED: Historical Context ---
            props_to_fill->oled_bottom_props.line1 = "--- History ---";
            // Extract just the date part (YYYYMMDD) from the timestamp for display
            std::string date_part = _last_cal_timestamp.substr(0, 8);
            snprintf(buffer, sizeof(buffer), "Last Cal: %s", date_part.c_str());
            props_to_fill->oled_bottom_props.line2 = buffer;
            snprintf(buffer, sizeof(buffer), "Cal Quality: %.1f %%", _cal_quality_score);
            props_to_fill->oled_bottom_props.line3 = buffer;
            
            // --- Buttons ---
            props_to_fill->button_props.back_text = "Done";
            props_to_fill->button_props.down_text = "Done";
            break;
    }
}

bool ProbeProfilingScreen::isAnalyzing() const {
    return _current_state == ProfilingState::ANALYZING;
}

uint8_t ProbeProfilingScreen::getSelectedAdcIndex() const {
    return _selected_index; // 0 for pH, 1 for EC
}

uint8_t ProbeProfilingScreen::getSelectedAdcInput() const {
    return ADS1118::DIFF_0_1;
}

const std::string& ProbeProfilingScreen::getSelectedFilterName() const {
    static const std::string ph_name = "ph_filter";
    static const std::string ec_name = "ec_filter";
    return (_selected_index == 0) ? ph_name : ec_name;
}

/**
 * @brief --- MODIFIED: Stores the full set of report card KPIs. ---
 */
void ProbeProfilingScreen::setAnalysisResults(double live_r_std, const PI_Filter& hfFilter, const PI_Filter& lfFilter, double zero_point_drift, double cal_quality_score, const std::string& last_cal_timestamp) {
    _live_r_std = live_r_std;
    _hf_params_snapshot = hfFilter; // Uses safe copy assignment
    _lf_params_snapshot = lfFilter; // Uses safe copy assignment
    _zero_point_drift = zero_point_drift;
    _cal_quality_score = cal_quality_score;
    _last_cal_timestamp = last_cal_timestamp;
    _current_state = ProfilingState::VIEW_REPORT;
}

void ProbeProfilingScreen::handleSelectProbeInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _current_state = ProfilingState::ANALYZING;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MAINTENANCE_MENU);
    }
}

void ProbeProfilingScreen::handleViewReportInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_DOWN_PRESS) {
        _current_state = ProfilingState::SELECT_PROBE;
    }
}