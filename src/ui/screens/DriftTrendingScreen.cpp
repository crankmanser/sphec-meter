// File Path: /src/ui/screens/DriftTrendingScreen.cpp
// MODIFIED FILE

#include "DriftTrendingScreen.h"
#include "pBiosContext.h"
#include "AdcManager.h"
#include "ADS1118.h"
#include <stdio.h>
#include <algorithm>
#include "ui/UIManager.h"

DriftTrendingScreen::DriftTrendingScreen(PBiosContext* context, AdcManager* adcManager) :
    _context(context),
    _adcManager(adcManager),
    _current_state(TrendingState::SELECT_SOURCE),
    _selected_source_index(0),
    _selected_duration_index(0),
    _sampling_progress_percent(0)
{
    _source_menu_items.push_back("pH Probe");
    _source_menu_items.push_back("EC Probe");
    _duration_menu_items.push_back("30 Seconds");
    _duration_values_sec.push_back(30);
    _duration_menu_items.push_back("60 Seconds");
    _duration_values_sec.push_back(60);
    _duration_menu_items.push_back("120 Seconds");
    _duration_values_sec.push_back(120);
    for (int i = 0; i < FFT_BIN_COUNT; ++i) {
        _fft_results[i] = 0.0;
    }
}

void DriftTrendingScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _current_state = TrendingState::SELECT_SOURCE;
    _sampling_progress_percent = 0;
}

/**
 * @brief --- NEW: Implements the onExit fail-safe. ---
 * Deactivates the currently selected probe to ensure it doesn't stay on
 * if the user leaves this screen.
 */
void DriftTrendingScreen::onExit() {
    if (_context && _adcManager) {
        _adcManager->setProbeState(_context->selectedAdcIndex, ProbeState::DORMANT);
    }
}

void DriftTrendingScreen::handleInput(const InputEvent& event) {
    switch (_current_state) {
        case TrendingState::SELECT_SOURCE: handleSelectSourceInput(event); break;
        case TrendingState::SELECT_DURATION: handleSelectDurationInput(event); break;
        case TrendingState::VIEW_RESULTS: handleViewResultsInput(event); break;
        case TrendingState::SAMPLING: case TrendingState::ANALYZING: break;
    }
}

void DriftTrendingScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    switch (_current_state) {
        case TrendingState::SELECT_SOURCE: getSelectSourceRenderProps(props_to_fill); break;
        case TrendingState::SELECT_DURATION: getSelectDurationRenderProps(props_to_fill); break;
        case TrendingState::SAMPLING: getSamplingRenderProps(props_to_fill); break;
        case TrendingState::ANALYZING: getAnalyzingRenderProps(props_to_fill); break;
        case TrendingState::VIEW_RESULTS: getViewResultsRenderProps(props_to_fill); break;
    }
}

void DriftTrendingScreen::handleSelectSourceInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_source_index < _source_menu_items.size() - 1) _selected_source_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_source_index > 0) _selected_source_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        if (_context) {
            _context->selectedAdcIndex = _selected_source_index;
            _context->selectedAdcInput = ADS1118::DIFF_0_1;
        }
        _current_state = TrendingState::SELECT_DURATION;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::PBIOS_MENU);
    }
}

void DriftTrendingScreen::handleSelectDurationInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_duration_index < _duration_menu_items.size() - 1) _selected_duration_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_duration_index > 0) _selected_duration_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        // --- DEFINITIVE FIX: Activate the selected probe right before sampling. ---
        if (_context && _adcManager) {
            _adcManager->setProbeState(_context->selectedAdcIndex, ProbeState::ACTIVE);
        }
        _sampling_progress_percent = 0;
        _current_state = TrendingState::SAMPLING;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        _current_state = TrendingState::SELECT_SOURCE;
    }
}

void DriftTrendingScreen::handleViewResultsInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_DOWN_PRESS) {
        // --- DEFINITIVE FIX: Deactivate the probe after viewing results. ---
        if (_context && _adcManager) {
            _adcManager->setProbeState(_context->selectedAdcIndex, ProbeState::DORMANT);
        }
        _current_state = TrendingState::SELECT_SOURCE;
    }
}

void DriftTrendingScreen::getSelectSourceRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Drift Trending";
    props_to_fill->oled_bottom_props.line1 = "Step 1: Select signal source.";
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _source_menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_source_index;
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Next";
}

void DriftTrendingScreen::getSelectDurationRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Source: " + _source_menu_items[_selected_source_index];
    props_to_fill->oled_bottom_props.line1 = "Step 2: Select analysis duration.";
    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _duration_menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_duration_index;
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Analyze";
}

void DriftTrendingScreen::getSamplingRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Drift Trending Analysis";
    props_to_fill->oled_bottom_props.line1 = "Acquiring long-duration data...";
    props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
    props_to_fill->oled_middle_props.progress_bar_props.label = "Sampling...";
    props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _sampling_progress_percent;
}

void DriftTrendingScreen::getAnalyzingRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "Drift Trending Analysis";
    props_to_fill->oled_middle_props.line1 = "Analyzing...";
    props_to_fill->oled_middle_props.line2 = "Performing FFT...";
    props_to_fill->oled_bottom_props.line1 = "Please wait.";
}

void DriftTrendingScreen::getViewResultsRenderProps(UIRenderProps* props_to_fill) {
    char buffer[32];
    props_to_fill->oled_top_props.line1 = "FFT Results for:";
    props_to_fill->oled_top_props.line2 = _source_menu_items[_selected_source_index];
    props_to_fill->oled_top_props.line3 = "Duration: " + _duration_menu_items[_selected_duration_index];
    props_to_fill->oled_middle_props.line1 = "Dominant Frequencies:";
    int peak_indices[3] = {-1, -1, -1};
    double peak_mags[3] = {0, 0, 0};
    for (int i = 1; i < FFT_BIN_COUNT; i++) {
        if (_fft_results[i] > peak_mags[0]) {
            peak_mags[2] = peak_mags[1]; peak_indices[2] = peak_indices[1];
            peak_mags[1] = peak_mags[0]; peak_indices[1] = peak_indices[0];
            peak_mags[0] = _fft_results[i]; peak_indices[0] = i;
        } else if (_fft_results[i] > peak_mags[1]) {
            peak_mags[2] = peak_mags[1]; peak_indices[2] = peak_indices[1];
            peak_mags[1] = _fft_results[i]; peak_indices[1] = i;
        } else if (_fft_results[i] > peak_mags[2]) {
            peak_mags[2] = _fft_results[i]; peak_indices[2] = i;
        }
    }
    float sample_rate_hz = (float)DRIFT_SAMPLE_COUNT / getSelectedDurationSec();
    float f1_hz = (float)peak_indices[0] * sample_rate_hz / DRIFT_SAMPLE_COUNT;
    snprintf(buffer, sizeof(buffer), "1: %.3f Hz (%.2f)", f1_hz, peak_mags[0]);
    props_to_fill->oled_middle_props.line2 = buffer;
    float f2_hz = (float)peak_indices[1] * sample_rate_hz / DRIFT_SAMPLE_COUNT;
    snprintf(buffer, sizeof(buffer), "2: %.3f Hz (%.2f)", f2_hz, peak_mags[1]);
    props_to_fill->oled_middle_props.line3 = buffer;
    props_to_fill->oled_bottom_props.line1 = "Press any button to continue.";
    props_to_fill->button_props.back_text = "Done";
}

void DriftTrendingScreen::setAnalyzing() {
    _current_state = TrendingState::ANALYZING;
}

void DriftTrendingScreen::setAnalysisResults(const double* fft_magnitudes) {
    for (int i = 0; i < FFT_BIN_COUNT; ++i) {
        _fft_results[i] = fft_magnitudes[i];
    }
    _current_state = TrendingState::VIEW_RESULTS;
}

void DriftTrendingScreen::setSamplingProgress(int percent) {
    _sampling_progress_percent = percent;
}

bool DriftTrendingScreen::isSampling() const {
    return _current_state == TrendingState::SAMPLING;
}

int DriftTrendingScreen::getSelectedDurationSec() const {
    if (_selected_duration_index < _duration_values_sec.size()) {
        return _duration_values_sec[_selected_duration_index];
    }
    return 0;
}