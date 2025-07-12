// src/managers/io/ButtonManager.cpp
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

bool ButtonManager::isPressed(ButtonType btn) {
    return getState(btn) == STATE_PRESSED;
}