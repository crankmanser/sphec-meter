// File Path: /src/ui/screens/PowerOffScreen.cpp
// NEW FILE

#include "PowerOffScreen.h"
#include "ui/UIManager.h" // For UIRenderProps definition
#include <esp_system.h>  // For esp_restart()

PowerOffScreen::PowerOffScreen() {}

/**
 * @brief Handles input for the power off screen.
 * The only action is to reboot the device if the user presses a button.
 */
void PowerOffScreen::handleInput(const InputEvent& event) {
    if (event.type == InputEventType::BTN_DOWN_PRESS) {
        esp_restart();
    }
}

/**
 * @brief Gets the render properties for the power off screen.
 * Displays a clear message across all three screens.
 */
void PowerOffScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line2 = "All settings have been";
    props_to_fill->oled_top_props.line3 = "saved or discarded.";

    props_to_fill->oled_middle_props.line2 = "It is now safe to";
    props_to_fill->oled_middle_props.line3 = "power off the device.";

    props_to_fill->oled_bottom_props.line2 = "Thank you.";

    props_to_fill->button_props.down_text = "Reboot";
}