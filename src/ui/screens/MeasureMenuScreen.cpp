// File Path: /src/ui/screens/MeasureMenuScreen.cpp
// NEW FILE

#include "MeasureMenuScreen.h"
#include "ui/UIManager.h" // For UIRenderProps definition

MeasureMenuScreen::MeasureMenuScreen() : _selected_index(0) {
    _menu_items.push_back("pH Measurement");
    _menu_items.push_back("EC Measurement");
    _menu_items.push_back("Light Sensor");

    _menu_descriptions.push_back("View detailed live pH data.");
    _menu_descriptions.push_back("View detailed live EC data.");
    _menu_descriptions.push_back("View live ambient light levels.");
}

void MeasureMenuScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::ENCODER_INCREMENT) {
        if (_selected_index < _menu_items.size() - 1) _selected_index++;
    } else if (event.type == InputEventType::ENCODER_DECREMENT) {
        if (_selected_index > 0) _selected_index--;
    } else if (event.type == InputEventType::BTN_ENTER_PRESS) {
        if (_stateManager) {
            switch (_selected_index) {
                case 0: // pH
                    _stateManager->changeState(ScreenState::PROBE_MEASUREMENT, 0);
                    break;
                case 1: // EC
                    _stateManager->changeState(ScreenState::PROBE_MEASUREMENT, 1);
                    break;
                case 2: // Light Sensor
                    _stateManager->changeState(ScreenState::LIGHT_SENSOR);
                    break;
            }
        }
    } else if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MAIN_MENU);
    }
}

void MeasureMenuScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();

    if (_selected_index < _menu_descriptions.size()) {
        props_to_fill->oled_top_props.line1 = _menu_descriptions[_selected_index];
    }

    props_to_fill->oled_middle_props.menu_props.is_enabled = true;
    props_to_fill->oled_middle_props.menu_props.items = _menu_items;
    props_to_fill->oled_middle_props.menu_props.selected_index = _selected_index;

    props_to_fill->oled_bottom_props.line1 = "Main Menu > Measure";

    props_to_fill->button_props.back_text = "Back";
    props_to_fill->button_props.enter_text = "Select";
}