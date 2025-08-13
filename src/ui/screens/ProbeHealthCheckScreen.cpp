// File Path: /src/ui/screens/ProbeHealthCheckScreen.cpp
// NEW FILE

#include "ProbeHealthCheckScreen.h"
#include "ui/UIManager.h"
#include <stdio.h>

ProbeHealthCheckScreen::ProbeHealthCheckScreen() :
    _state(HealthCheckState::SELECT_PROBE),
    _selected_index(0),
    _live_stability_percent(0),
    _health_result_percent(0.0)
{
    _menu_items.push_back("pH Probe");
    _menu_items.push_back("EC Probe");
}

void ProbeHealthCheckScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _state = HealthCheckState::SELECT_PROBE;
}

void ProbeHealthCheckScreen::handleInput(const InputEvent& event) {
    if (_state == HealthCheckState::SELECT_PROBE) {
        if (event.type == InputEventType::ENCODER_INCREMENT) {
            if (_selected_index < _menu_items.size() - 1) _selected_index++;
        } else if (event.type == InputEventType::ENCODER_DECREMENT) {
            if (_selected_index > 0) _selected_index--;
        } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
            _state = HealthCheckState::MEASURING;
        } else if (event.type == InputEventType::BTN_BACK_PRESS) {
            if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
        }
    } else if (_state == HealthCheckState::MEASURING) {
        // STUB: Add logic for capturing when stable
    } else if (_state == HealthCheckState::VIEW_RESULT) {
        if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_ENTER_PRESS) {
             if (_stateManager) _stateManager->changeState(ScreenState::CALIBRATION_MENU);
        }
    }
}

void ProbeHealthCheckScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    char buffer[40];

    if (_state == HealthCheckState::SELECT_PROBE) {
        props_to_fill->oled_top_props.line1 = "1-Point Probe Health Check";
        props_to_fill->oled_middle_props.menu_props.is_enabled = true;
        props_to_fill->oled_middle_props.menu_props.items = _menu_items;
        props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;
        props_to_fill->oled_bottom_props.line1 = "Select probe to check.";
        props_to_fill->button_props.back_text = "Back";
        props_to_fill->button_props.enter_text = "Select";
    } else if (_state == HealthCheckState::MEASURING) {
        const char* ref = (_selected_index == 0) ? "pH 6.86" : "1413 uS";
        snprintf(buffer, sizeof(buffer), "Place probe in %s buffer", ref);
        props_to_fill->oled_top_props.line1 = buffer;
        props_to_fill->oled_middle_props.progress_bar_props.is_enabled = true;
        props_to_fill->oled_middle_props.progress_bar_props.label = "Stability";
        props_to_fill->oled_middle_props.progress_bar_props.progress_percent = _live_stability_percent;
        props_to_fill->button_props.down_text = (_live_stability_percent > 95) ? "Check Health" : "Wait...";
    } else if (_state == HealthCheckState::VIEW_RESULT) {
        props_to_fill->oled_top_props.line1 = "Health Check Result";
        snprintf(buffer, sizeof(buffer), "%s Health:", _menu_items[_selected_index].c_str());
        props_to_fill->oled_middle_props.line1 = buffer;
        snprintf(buffer, sizeof(buffer), "%.1f %%", _health_result_percent);
        props_to_fill->oled_middle_props.line2 = buffer; // Make this large later
        props_to_fill->button_props.enter_text = "Done";
    }
}