// File Path: /src/ui/screens/TempCalibrationScreen.cpp
// NEW FILE

#include "TempCalibrationScreen.h"
#include "ui/UIManager.h"
#include "TempManager.h"
#include <stdio.h>

extern TempManager tempManager;

TempCalibrationScreen::TempCalibrationScreen() :
    _probe_temp_live(0.0f),
    _ambient_temp_live(0.0f),
    _ref_temp(25.0f),
    _is_editing(false)
{}

void TempCalibrationScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _is_editing = false;
    // Initialize reference temperature to the current ambient reading
    _ref_temp = tempManager.getAmbientTemp();
}

void TempCalibrationScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_is_editing) {
            _is_editing = false;
        } else {
            if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
        }
    } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
        _is_editing = !_is_editing;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        // Save logic
        if (!_is_editing) {
            float probe_offset = _ref_temp - _probe_temp_live;
            float ambient_offset = _ref_temp - _ambient_temp_live;
            tempManager.setProbeTempOffset(probe_offset);
            tempManager.setAmbientTempOffset(ambient_offset);
            // STUB: Add logic to save these offsets to NVS
            if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
        }
    }

    if (_is_editing) {
        if (event.type == InputEventType::ENCODER_INCREMENT) {
            _ref_temp += 0.1f * event.value;
        } else if (event.type == InputEventType::ENCODER_DECREMENT) {
            _ref_temp -= 0.1f * event.value;
        }
    }
}

void TempCalibrationScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    _probe_temp_live = tempManager.getProbeTemp();
    _ambient_temp_live = tempManager.getAmbientTemp();
    char buffer[40];

    props_to_fill->oled_top_props.line1 = "Temperature Calibration";

    snprintf(buffer, sizeof(buffer), "Probe:   %.2f C", _probe_temp_live);
    props_to_fill->oled_middle_props.line1 = buffer;
    snprintf(buffer, sizeof(buffer), "Ambient: %.2f C", _ambient_temp_live);
    props_to_fill->oled_middle_props.line2 = buffer;
    snprintf(buffer, sizeof(buffer), "Ref Temp: %.2f C", _ref_temp);
    props_to_fill->oled_middle_props.line3 = buffer;
    
    if (_is_editing) {
        props_to_fill->oled_bottom_props.line1 = "> Editing Reference <";
    }

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = _is_editing ? "OK" : "Edit";
    props_to_fill->button_props.down_text = "Save";
}