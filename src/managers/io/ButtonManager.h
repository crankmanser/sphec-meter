// src/managers/io/ButtonManager.h
// MODIFIED FILE
#pragma once

#include <Arduino.h>

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
    // <<< MODIFIED: This is the new edge-detection method >>>
    bool wasJustPressed(ButtonType btn);

private:
    struct Button {
        const uint8_t pin;
        ButtonState state = STATE_RELEASED;
        uint32_t last_press_time = 0;
        bool is_held_registered = false;
        // <<< ADDED: Flag to track the single press event >>>
        bool just_pressed = false;

        Button(uint8_t p) : pin(p) {}
    };

    Button _btn_top;
    Button _btn_middle;
    Button _btn_bottom;
    
    static constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
    static constexpr uint32_t HOLD_DELAY_MS = 1000;

    void updateButton(Button& btn);
};