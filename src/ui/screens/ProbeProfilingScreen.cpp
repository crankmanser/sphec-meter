// File Path: /src/ui/screens/ProbeProfilingScreen.cpp
// MODIFIED FILE

#include "ProbeProfilingScreen.h"
#include "ADS1118.h"
#include <stdio.h>

ProbeProfilingScreen::ProbeProfilingScreen() :
    _current_state(ProfilingState::SELECT_PROBE),
    _selected_index(0),
    _live_r_std(0.0)
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
            break;
    }
}

/**
 * @brief --- DEFINITIVE FIX: Implements the clean, multi-screen report layout. ---
 * This function is completely rewritten to distribute the diagnostic data logically
 * across the three OLEDs, providing a clear and readable "report card" for the
 * selected probe and resolving all previous UI clutter and text overlap issues.
 */
void ProbeProfilingScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();

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
            char buffer[32];

            // --- Top OLED: Summary ---
            props_to_fill->oled_top_props.line1 = "Profile: " + _menu_items[_selected_index];
            snprintf(buffer, sizeof(buffer), "Live R_std: %.3f mV", _live_r_std);
            props_to_fill->oled_top_props.line2 = buffer;

            // --- Middle OLED: HF Filter Load ---
            props_to_fill->oled_middle_props.line1 = "--- HF Filter Load ---";
            snprintf(buffer, sizeof(buffer), "Settle: %.2f", _hf_params_snapshot.settleThreshold);
            props_to_fill->oled_middle_props.line2 = buffer;
            snprintf(buffer, sizeof(buffer), "Smooth: %.2f", _hf_params_snapshot.lockSmoothing);
            props_to_fill->oled_middle_props.line3 = buffer;
            
            // --- Bottom OLED: LF Filter Load ---
            props_to_fill->oled_bottom_props.line1 = "--- LF Filter Load ---";
            snprintf(buffer, sizeof(buffer), "Settle: %.3f", _lf_params_snapshot.settleThreshold);
            props_to_fill->oled_bottom_props.line2 = buffer;
            snprintf(buffer, sizeof(buffer), "Smooth: %.3f", _lf_params_snapshot.lockSmoothing);
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
    return _selected_index;
}

uint8_t ProbeProfilingScreen::getSelectedAdcInput() const {
    return ADS1118::DIFF_0_1;
}

const std::string& ProbeProfilingScreen::getSelectedFilterName() const {
    static const std::string ph_name = "ph_filter";
    static const std::string ec_name = "ec_filter";
    return (_selected_index == 0) ? ph_name : ec_name;
}

void ProbeProfilingScreen::setAnalysisResults(double live_r_std, const PI_Filter& hfFilter, const PI_Filter& lfFilter) {
    _live_r_std = live_r_std;
    _hf_params_snapshot = hfFilter;
    _lf_params_snapshot = lfFilter;
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