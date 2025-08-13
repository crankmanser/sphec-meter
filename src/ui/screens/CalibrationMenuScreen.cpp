// File Path: /src/ui/screens/CalibrationMenuScreen.cpp
// NEW FILE

#include "CalibrationMenuScreen.h"
#include "ui/UIManager.h" // For UIRenderProps definition
#include "ProbeMeasurementScreen.h" // For ProbeType enum

CalibrationMenuScreen::CalibrationMenuScreen() : _selected_index(0) {
    _menu_items.push_back("pH Probe (3-Point)");
    _menu_items.push_back("EC Probe (3-Point)");
    _menu_items.push_back("Probe Health Check (1-Point)");
    _menu_items.push_back("Temperature Sensors");
    _menu_items.push_back("Light Sensor");

    _menu_descriptions.push_back("Perform a full 3-point pH calibration.");
    _menu_descriptions.push_back("Perform a full 3-point EC calibration.");
    _menu_descriptions.push_back("Perform a quick 1-point health check.");
    _menu_descriptions.push_back("Calibrate temperature sensors.");
    _menu_descriptions.push_back("Calibrate the ambient light sensor.");
}

void CalibrationMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
        if (_stateManager) {
            switch (_selected_index) {
                case 0: // pH 3-Point
                    _stateManager->changeState(ScreenState::CALIBRATION_WIZARD, static_cast<int>(ProbeType::PH));
                    break;
                case 1: // EC 3-Point
                    _stateManager->changeState(ScreenState::CALIBRATION_WIZARD, static_cast<int>(ProbeType::EC));
                    break;
                case 2: // 1-Point Health Check
                    _stateManager->changeState(ScreenState::PROBE_HEALTH_CHECK);
                    break;
                case 3: // Temperature
                    _stateManager->changeState(ScreenState::TEMP_CALIBRATION);
                    break;
                case 4: // Light Sensor (Placeholder)
                    //_stateManager->changeState(ScreenState::LIGHT_SENSOR_CAL);
                    break;
            }
        }
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MAIN_MENU);
    }
}

void CalibrationMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();

    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.line1 = "Main Menu > Calibrate";

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "Select";
}