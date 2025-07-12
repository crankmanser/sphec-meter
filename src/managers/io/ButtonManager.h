// src/managers/io/ButtonManager.h
// MODIFIED FILE
#pragma once

#include <Arduino.h>

/**
 * @class ButtonManager
 * @brief Manages the physical buttons, handling debouncing and press/hold detection.
 *
 * This is a refactor of the robust legacy ButtonManager, responsible for turning
 * raw pin states into clean, logical button events.
 */
class ButtonManager {
public:
    enum ButtonType {
        BTN_TOP,
        BTN_MIDDLE,
        BTN_BOTTOM
    };

    enum ButtonState {
        STATE_RELEASED,
        STATE_PRESSED,
        STATE_HELD
    };

    ButtonManager(uint8_t pin_top, uint8_t pin_middle, uint8_t pin_bottom);
    void begin();
    void update(); // Should be called frequently in a high-priority task.

    ButtonState getState(ButtonType btn);
    bool isPressed(ButtonType btn);

private:
    struct Button {
        const uint8_t pin;
        ButtonState state = STATE_RELEASED;
        uint32_t last_press_time = 0;
        bool is_held_registered = false;

        // <<< FIX: Explicit constructor to initialize the const 'pin' member.
        Button(uint8_t p) : pin(p) {}
    };

    Button _btn_top;
    Button _btn_middle;
    Button _btn_bottom;
    
    static constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
    static constexpr uint32_t HOLD_DELAY_MS = 1000;

    void updateButton(Button& btn);
};