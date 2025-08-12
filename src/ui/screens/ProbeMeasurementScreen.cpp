// File Path: /src/ui/screens/ProbeMeasurementScreen.cpp
// MODIFIED FILE

#include "ProbeMeasurementScreen.h"
#include "ui/UIManager.h"
#include "AdcManager.h"
#include "CalibrationManager.h"
#include "TempManager.h"
#include <stdio.h>

// Make global managers accessible to this screen
extern AdcManager adcManager;
extern CalibrationManager phCalManager, ecCalManager;
extern TempManager tempManager;

ProbeMeasurementScreen::ProbeMeasurementScreen() :
    _probe_type(ProbeType::PH),
    _capture_requested(false),
    _calibrated_value(NAN),
    _temperature(NAN),
    _stability_percent(0),
    _raw_millivolts(0.0),
    _filtered_millivolts(0.0)
{}

void ProbeMeasurementScreen::onEnter(StateManager* stateManager, int probe_type_int) {
    Screen::onEnter(stateManager);
    _probe_type = static_cast<ProbeType>(probe_type_int);
    _capture_requested = false;

    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::ACTIVE);
}

void ProbeMeasurementScreen::onExit() {
    uint8_t probe_index = (_probe_type == ProbeType::PH) ? 0 : 1;
    adcManager.setProbeState(probe_index, ProbeState::DORMANT);
}

void ProbeMeasurementScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MEASURE_MENU);
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _capture_requested = true;
    }
}

/**
 * @brief --- DEFINITIVE FIX: Reorganizes the layout to match the user's photo. ---
 * All data points on the top and bottom OLEDs are now on their own separate lines
 * for maximum clarity and readability.
 */
void ProbeMeasurementScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    char buffer[40];
    const char* unit = (_probe_type == ProbeType::PH) ? "pH" : "uS";

    // --- Top OLED: Primary Reading ---
    props_to_fill->oled_top_props.line1 = (_probe_type == ProbeType::PH) ? "pH Value" : "EC Value";
    if (isnan(_calibrated_value)) {
        snprintf(buffer, sizeof(buffer), "--- %s", unit);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f %s", _calibrated_value, unit);
    }
    props_to_fill->oled_top_props.line2 = buffer;
    snprintf(buffer, sizeof(buffer), "Temp: %.1f C", _temperature);
    props_to_fill->oled_top_props.line3 = buffer;
    snprintf(buffer, sizeof(buffer), "Stab: %d%%", _stability_percent);
    props_to_fill->oled_top_props.line4 = buffer;


    // --- Middle OLED: Calibration Curve Graph ---
    props_to_fill->oled_middle_props.cal_curve_props.is_enabled = true;
    props_to_fill->oled_middle_props.cal_curve_props.model = (_probe_type == ProbeType::PH) ? &phCalManager.getCurrentModel() : &ecCalManager.getCurrentModel();
    props_to_fill->oled_middle_props.cal_curve_props.live_voltage = _filtered_millivolts;
    props_to_fill->oled_middle_props.cal_curve_props.live_value = _calibrated_value;


    // --- Bottom OLED: Raw Data & Ambient (Corrected Layout) ---
    snprintf(buffer, sizeof(buffer), "Raw: %.2f mV", _raw_millivolts);
    props_to_fill->oled_bottom_props.line1 = buffer;
    snprintf(buffer, sizeof(buffer), "Filt: %.2f mV", _filtered_millivolts);
    props_to_fill->oled_bottom_props.line2 = buffer;
    snprintf(buffer, sizeof(buffer), "Ambient: %.1f C", tempManager.getAmbientTemp());
    props_to_fill->oled_bottom_props.line3 = buffer;
    snprintf(buffer, sizeof(buffer), "RH: %.0f %%", tempManager.getHumidity());
    props_to_fill->oled_bottom_props.line4 = buffer;


    // --- Button Prompts ---
    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.down_text = "Capture";
}


void ProbeMeasurementScreen::updateData(double calibrated_value, double temp, int stability, double raw_mv, double filtered_mv) {
    _calibrated_value = calibrated_value;
    _temperature = temp;
    _stability_percent = stability;
    _raw_millivolts = raw_mv;
    _filtered_millivolts = filtered_mv;
}

bool ProbeMeasurementScreen::captureWasRequested() {
    return _capture_requested;
}

void ProbeMeasurementScreen::clearCaptureRequest() {
    _capture_requested = false;
}