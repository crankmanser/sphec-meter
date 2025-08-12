// File Path: /src/ui/screens/LightSensorScreen.cpp
// NEW FILE

#include "LightSensorScreen.h"
#include "ui/UIManager.h" // For UIRenderProps definition

LightSensorScreen::LightSensorScreen() {}

void LightSensorScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_BACK_PRESS) {
        if (_stateManager) _stateManager->changeState(ScreenState::MEASURE_MENU);
    }
}

void LightSensorScreen::getRenderProps(UIRenderProps* props_to_fill) {
    *props_to_fill = UIRenderProps();
    props_to_fill->oled_top_props.line1 = "Light Sensor";
    props_to_fill->oled_middle_props.line2 = "Feature not yet";
    props_to_fill->oled_middle_props.line3 = "implemented.";
    props_to_fill->button_props.back_text = "Back";
}