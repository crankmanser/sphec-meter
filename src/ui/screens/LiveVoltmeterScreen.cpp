// File Path: /src/ui/screens/LiveVoltmeterScreen.cpp
// MODIFIED FILE

#include "LiveVoltmeterScreen.h"
#include "ADS1118.h" // For ADC channel definitions
#include <stdio.h>   // For snprintf
#include "ui/UIManager.h" // Include for UIRenderProps definition

LiveVoltmeterScreen::LiveVoltmeterScreen() :
    _current_state(VoltmeterState::SELECT_SOURCE),
    _selected_index(0),
    _live_voltage_mv(0.0) 
{
    _source_menu_items.push_back("pH Probe");
    _source_menu_items.push_back("EC Probe");
    _source_menu_items.push_back("3.3V Bus");
    _source_menu_items.push_back("5.0V Bus");
}

/**
 * @brief --- DEFINITIVE FIX: Update signature to match the base class ---
 */
void LiveVoltmeterScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    // Reset to the selection state every time the user enters this screen.
    _current_state = VoltmeterState::SELECT_SOURCE;
    _live_voltage_mv = 0.0;
}

// ... (rest of the file is unchanged) ...
void LiveVoltmeterScreen::handleInput(const InputEvent& event) {
    if (_current_state == VoltmeterState::SELECT_SOURCE) {
        handleSelectSourceInput(event);
    } else {
        handleMeasuringInput(event);
    }
}
void LiveVoltmeterScreen::getRenderProps(UIRenderProps* props_to_fill) {
    if (_current_state == VoltmeterState::SELECT_SOURCE) {
        props_to_fill->oled_top_props.line1 = "pBios > ADC Voltmeter";
        props_to_fill->oled_middle_props.menu_props.is_enabled = true;
        props_to_fill->oled_middle_props.menu_props.items = _source_menu_items;
        props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;
        props_to_fill->oled_bottom_props.line1 = "Select source to measure.";
        props_to_fill->button_props.back_text = "Back";
        props_to_fill->button_props.down_text = "Measure";
    } else {
        char buffer[32];
        props_to_fill->oled_top_props.line1 = "Live Voltmeter";
        props_to_fill->oled_top_props.line2 = "Source: " + _source_menu_items[_selected_index];
        
        snprintf(buffer, sizeof(buffer), "%.2f mV", _live_voltage_mv);
        props_to_fill->oled_middle_props.line2 = "Reading:";
        props_to_fill->oled_middle_props.line3 = buffer;
        
        props_to_fill->oled_bottom_props.line1 = "Press Back to stop.";
        props_to_fill->button_props.back_text = "Back";
    }
}
void LiveVoltmeterScreen::setLiveVoltage(double voltage) {
    _live_voltage_mv = voltage;
}
uint8_t LiveVoltmeterScreen::getSelectedAdcIndex() const {
    // pH and 3.3V are on ADC 0, EC and 5.0V are on ADC 1
    return (_selected_index == 1 || _selected_index == 3) ? 1 : 0;
}
uint8_t LiveVoltmeterScreen::getSelectedAdcInput() const {
    switch (_selected_index) {
        case 0: return ADS1118::DIFF_0_1; // pH
        case 1: return ADS1118::DIFF_0_1; // EC
        case 2: return ADS1118::AIN_2;     // 3.3V
        case 3: return ADS1118::AIN_2;     // 5.0V
        default: return 0;
    }
}
bool LiveVoltmeterScreen::isMeasuring() const {
    return _current_state == VoltmeterState::MEASURING;
}
void LiveVoltmeterScreen::handleSelectSourceInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _source_menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_DOWN_PRESS) {
        _current_state = VoltmeterState::MEASURING;
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MAINTENANCE_MENU);
    }
}
void LiveVoltmeterScreen::handleMeasuringInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        _current_state = VoltmeterState::SELECT_SOURCE;
    }
}