// src/managers/io/ButtonManager.cpp
// MODIFIED FILE
#include "managers/io/ButtonManager.h"

ButtonManager::ButtonManager(uint8_t pin_top, uint8_t pin_middle, uint8_t pin_bottom) :
    _btn_top{pin_top},
    _btn_middle{pin_middle},
    _btn_bottom{pin_bottom}
{}

void ButtonManager::begin() {
    pinMode(_btn_top.pin, INPUT_PULLUP);
    pinMode(_btn_middle.pin, INPUT_PULLUP);
    pinMode(_btn_bottom.pin, INPUT_PULLUP);
}

void ButtonManager::update() {
    updateButton(_btn_top);
    updateButton(_btn_middle);
    updateButton(_btn_bottom);
}

void ButtonManager::updateButton(Button& btn) {
    bool is_currently_pressed = (digitalRead(btn.pin) == LOW);
    uint32_t now = millis();

    switch (btn.state) {
        case STATE_RELEASED:
            if (is_currently_pressed) {
                btn.state = STATE_PRESSED;
                btn.last_press_time = now;
                // <<< ADDED: Set the just_pressed flag on the transition >>>
                btn.just_pressed = true; 
            }
            break;
        
        case STATE_PRESSED:
            if (!is_currently_pressed) {
                btn.state = STATE_RELEASED;
            } else if (!btn.is_held_registered && (now - btn.last_press_time > HOLD_DELAY_MS)) {
                btn.state = STATE_HELD;
                btn.is_held_registered = true;
            }
            break;

        case STATE_HELD:
            if (!is_currently_pressed) {
                btn.state = STATE_RELEASED;
                btn.is_held_registered = false;
            }
            break;
    }
}

ButtonManager::ButtonState ButtonManager::getState(ButtonType btn) {
    switch (btn) {
        case BTN_TOP:    return _btn_top.state;
        case BTN_MIDDLE: return _btn_middle.state;
        case BTN_BOTTOM: return _btn_bottom.state;
    }
    return STATE_RELEASED;
}

// <<< ADDED: Implementation for the new edge-detection method >>>
bool ButtonManager::wasJustPressed(ButtonType btn) {
    Button* p_btn = nullptr;
    switch (btn) {
        case BTN_TOP:    p_btn = &_btn_top;    break;
        case BTN_MIDDLE: p_btn = &_btn_middle; break;
        case BTN_BOTTOM: p_btn = &_btn_bottom; break;
    }

    if (p_btn && p_btn->just_pressed) {
        // Read the flag, clear it, and return true.
        p_btn->just_pressed = false;
        return true;
    }
    return false;
}