// File Path: /src/ui/screens/CalibrationWizardScreen.cpp
// MODIFIED FILE

#include "CalibrationWizardScreen.h"
#include "ui/UIManager.h"
#include "AdcManager.h"
#include "CalibrationManager.h" // --- FIX: Include the required header ---
#include <stdio.h>

// --- FIX: Declare the external global managers so this file can see them ---
extern AdcManager adcManager;
extern CalibrationManager phCalManager, ecCalManager;

/**
 * @brief Constructor for the CalibrationWizardScreen.
 * Initializes all member variables to their default states.
 * @version 3.1.11
 */
CalibrationWizardScreen::CalibrationWizardScreen() :
    _probe_type(ProbeType::PH),
    _wizard_state(WizardState::INTRODUCTION),
    _current_step(1),
    _live_stability_percent(0),
    _point_capture_requested(false), // Initialize new flag
    _save_requested(false),
    _result_quality_score(0.0),
    _result_sensor_drift(0.0)
{}

/**
 * @brief Called when the screen becomes active.
 * Resets the wizard to its initial state and activates the correct probe.
 * @param stateManager Pointer to the global state manager.
 * @param context Integer context, used here to determine the probe type.
 */
void CalibrationWizardScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _probe_type = static_cast<ProbeType>(context);
    _wizard_state = WizardState::INTRODUCTION;
    _current_step = 1;
    _live_stability_percent = 0;
    _point_capture_requested = false; // Reset on entry
    _save_requested = false;

    // Activate the appropriate probe's power supply
    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
}

/**
 * @brief Called when the screen is exited.
 * Ensures the probe is returned to a low-power (dormant) state.
 */
void CalibrationWizardScreen::onExit() {
    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::DORMANT);
}

/**
 * @brief Main input handler, delegates to state-specific handlers.
 * @param event The input event to process.
 */
void CalibrationWizardScreen::handleInput(const InputEvent& event) {
    switch (_wizard_state) {
        case WizardState::INTRODUCTION:  handleIntroInput(event); break;
        case WizardState::MEASURE_POINT: handleMeasurePointInput(event); break;
        case WizardState::CALCULATING:   /* No user input during calculation */ break;
        case WizardState::VIEW_RESULTS:  handleResultsInput(event); break;
    }
}

/**
 * @brief Populates the UIRenderProps structure based on the current wizard state.
 * @param props_to_fill A pointer to the structure to be filled with rendering properties.
 */
void CalibrationWizardScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    char buffer[40];
    const char* probe_name = (_probe_type == ProbeType::PH) ? "pH" : "EC";
    // Define the known values for the calibration points for both probe types
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
            // The button is only enabled when the signal is stable
            props_to_fill->button_props.back_text = "Cancel";
            props_to_fill->button_props.down_text = (_live_stability_percent > 95) ? "Capture" : "Wait...";
            break;

        case WizardState::CALCULATING:
            props_to_fill->oled_top_props.line1 = "Calibration";
            props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
            props_to_fill->oled_middle_props.progress_bar_props.label = "Calculating model...";
            props_to_fill->oled_middle_props.progress_bar_props.progress_percent = 50; // Indeterminate progress
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

// --- Public Methods for Backend (dataTask) Interaction ---

bool CalibrationWizardScreen::isMeasuring() const { return _wizard_state == WizardState::MEASURE_POINT; }
bool CalibrationWizardScreen::isCalculating() const { return _wizard_state == WizardState::CALCULATING; }

bool CalibrationWizardScreen::pointCaptureWasRequested() { return _point_capture_requested; }
void CalibrationWizardScreen::clearPointCaptureRequest() { _point_capture_requested = false; }

bool CalibrationWizardScreen::saveWasRequested() { return _save_requested; }
void CalibrationWizardScreen::clearSaveRequest() { _save_requested = false; }

void CalibrationWizardScreen::setLiveStability(int percent) { _live_stability_percent = percent; }

void CalibrationWizardScreen::setResults(double quality_score, double sensor_drift) {
    _result_quality_score = quality_score;
    _result_sensor_drift = sensor_drift;
    _wizard_state = WizardState::VIEW_RESULTS; // Transition UI to the results view
}

void CalibrationWizardScreen::advanceToNextStep() {
    if (_wizard_state == WizardState::MEASURE_POINT) {
        _current_step++;
        _live_stability_percent = 0; // Reset stability for the next point
    }
}

void CalibrationWizardScreen::transitionToCalculating() {
    _wizard_state = WizardState::CALCULATING;
}


// --- Private Input Handlers ---

void CalibrationWizardScreen::handleIntroInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _wizard_state = WizardState::MEASURE_POINT;
        // The dataTask needs to know to start the calibration process
        phCalManager.startNewCalibration();
        ecCalManager.startNewCalibration();

    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}

/**
 * @brief Input handler for the measurement step.
 * Sets the capture flag when the user presses the button and the signal is stable.
 * @version 3.1.11
 */
void CalibrationWizardScreen::handleMeasurePointInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS && _live_stability_percent > 95) {
        // --- MODIFIED: Signal the backend instead of changing state directly ---
        _point_capture_requested = true;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}

void CalibrationWizardScreen::handleResultsInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_ENTER_PRESS) { // "Save" button
        _save_requested = true; // Signal the backend to save the new model
    } else if (event.type == InputEventType::BTN_BACK_PRESS) { // "Discard" button
        if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
    }
}