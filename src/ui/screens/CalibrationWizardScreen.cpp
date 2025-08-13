// File Path: /src/ui/screens/CalibrationWizardScreen.cpp
// MODIFIED FILE

#include "CalibrationWizardScreen.h"
#include "ui/UIManager.h"
#include "AdcManager.h"
#include <stdio.h>

extern AdcManager adcManager; // Make global AdcManager available

CalibrationWizardScreen::CalibrationWizardScreen() :
    _probe_type(ProbeType::PH),
    _wizard_state(WizardState::INTRODUCTION),
    _current_step(1),
    _live_stability_percent(0),
    _save_requested(false),
    _result_quality_score(0.0),
    _result_sensor_drift(0.0)
{}

void CalibrationWizardScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _probe_type = static_cast<ProbeType>(context);
    _wizard_state = WizardState::INTRODUCTION;
    _current_step = 1;
    _live_stability_percent = 0;
    _save_requested = false;

    // --- NEW: Activate the probe when the wizard starts ---
    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
}

/**
 * @brief --- NEW: Deactivates the probe when the wizard is exited. ---
 * Ensures the probe is returned to a low-power state regardless of how
 * the user exits the calibration process.
 */
void CalibrationWizardScreen::onExit() {
    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::DORMANT);
}


void CalibrationWizardScreen::handleInput(const InputEvent& event) {
    switch (_wizard_state) {
        case WizardState::INTRODUCTION:  handleIntroInput(event); break;
        case WizardState::MEASURE_POINT: handleMeasurePointInput(event); break;
        case WizardState::CALCULATING:   break;
        case WizardState::VIEW_RESULTS:  handleResultsInput(event); break;
    }
}

void CalibrationWizardScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    char buffer[40];
    const char* probe_name = (_probe_type == ProbeType::PH) ? "pH" : "EC";
    const char* points[3] = {"4.01", "6.86", "9.18"};
    if (_probe_type == ProbeType::EC) {
        points[0] = "84 uS"; points[1] = "1413 uS"; points[2] = "12.88 mS";
    }

    switch (_wizard_state) {
        case WizardState::INTRODUCTION:
            snprintf(buffer, sizeof(buffer), "%s 3-Point Calibration", probe_name);
            props_to_fill->oled_top_props.line1 = buffer;
            props_to_fill->oled_middle_props.line1 = "Prepare your 3 buffer";
            props_to_fill->oled_middle_props.line2 = "solutions and press Begin.";
            props_to_fill->button_props.back_text = "Cancel";
            props_to_fill->button_props.down_text = "Begin";
            break;
        case WizardState::MEASURE_POINT:
            snprintf(buffer, sizeof(buffer), "Step %d/3: Measure %s", _current_step, points[_current_step - 1]);
            props_to_fill->oled_top_props.line1 = buffer;
            props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
            props_to_fill->oled_middle_props.progress_bar_props.label = "Stability";
            props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _live_stability_percent;
            props_to_fill->button_props.back_text = "Cancel";
            props_to_fill->button_props.down_text = (_live_stability_percent > 95) ? "Capture" : "Wait...";
            break;
        case WizardState::CALCULATING:
            props_to_fill->oled_top_props.line1 = "Calibration";
            props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
            props_to_fill->oled_middle_props.progress_bar_props.label = "Calculating model...";
            props_to_fill->oled_middle_props.progress_bar_props.progress_percent = 50; 
            break;
        case WizardState::VIEW_RESULTS:
            props_to_fill->oled_top_props.line1 = "Calibration Results";
            snprintf(buffer, sizeof(buffer), "Quality Score: %.1f %%", _result_quality_score);
            props_to_fill->oled_middle_props.line1 = buffer;
            snprintf(buffer, sizeof(buffer), "Sensor Drift: %.2f %%", _result_sensor_drift);
            props_to_fill->oled_middle_props.line2 = buffer;
            props_to_fill->oled_bottom_props.line1 = "Save new calibration?";
            props_to_fill->button_props.back_text = "Discard";
            props_to_fill->button_props.enter_text = "Save";
            break;
    }
}

bool CalibrationWizardScreen::isMeasuring() const {
    return _wizard_state == WizardState::MEASURE_POINT;
}

bool CalibrationWizardScreen::isCalculating() const {
    return _wizard_state == WizardState::CALCULATING;
}

bool CalibrationWizardScreen::saveWasRequested() {
    return _save_requested;
}

void CalibrationWizardScreen::clearSaveRequest() {
    _save_requested = false;
}

void CalibrationWizardScreen::setLiveStability(int percent) {
    _live_stability_percent = percent;
}

void CalibrationWizardScreen::setResults(double quality_score, double sensor_drift) {
    _result_quality_score = quality_score;
    _result_sensor_drift = sensor_drift;
    _wizard_state = WizardState::VIEW_RESULTS;
}

void CalibrationWizardScreen::handleIntroInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _wizard_state = WizardState::MEASURE_POINT;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}

void CalibrationWizardScreen::handleMeasurePointInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS && _live_stability_percent > 95) {
        _current_step++;
        _live_stability_percent = 0;
        if (_current_step > 3) {
            _wizard_state = WizardState::CALCULATING;
        }
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}

void CalibrationWizardScreen::handleResultsInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_ENTER_PRESS) { // Save
        _save_requested = true;
        // The dataTask will see this, save the model, and then transition state
    } else if (event.type == InputEventType::BTN_BACK_PRESS) { // Discard
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}