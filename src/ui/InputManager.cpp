// File Path: /src/ui/InputManager.cpp
// MODIFIED FILE

#include "InputManager.h"
#include <ProjectConfig.h>

volatile int InputManager::_encoder_raw_pulses = 0;

// --- Encoder Velocity Engine Tuning ---
namespace EncoderTuning {
    // --- FIX: Using your exact, tested tuning values ---
    constexpr int PULSES_PER_DETENT = 70;
    constexpr int PULSES_PER_STEP_SLOW = PULSES_PER_DETENT * 24;
    constexpr int PULSES_PER_STEP_MEDIUM = PULSES_PER_DETENT * 12;
    constexpr int PULSES_PER_STEP_FAST = PULSES_PER_DETENT * 6;
    constexpr int SPEED_THRESHOLD_MEDIUM = 20;
    constexpr int SPEED_THRESHOLD_FAST = 40;
}

void IRAM_ATTR InputManager::encoderISR() {
    static uint8_t last_state = 0;
    static const int8_t QEM_DECODE_TABLE[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    uint8_t current_state = (digitalRead(ENCODER_PIN_B) << 1) | digitalRead(ENCODER_PIN_A);
    _encoder_raw_pulses += QEM_DECODE_TABLE[(last_state << 2) | current_state];
    last_state = current_state;
}

InputManager::InputManager() :
    _back_pressed(false), _enter_pressed(false), _down_pressed(false),
    _back_last_state(false), _back_last_debounce_time(0),
    _enter_last_state(false), _enter_last_debounce_time(0),
    _down_last_state(false), _down_last_debounce_time(0),
    _encoder_change(0)
{}

void InputManager::begin() {
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(BTN_BACK_PIN, INPUT_PULLUP);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);
}

void InputManager::update() {
    uint32_t now = millis();
    _back_pressed = false;
    _enter_pressed = false;
    _down_pressed = false;
    _encoder_change = 0;

    // --- FIX: Robust edge-detection for button presses (active LOW) ---
    bool back_reading = (digitalRead(BTN_BACK_PIN) == LOW);
    if (back_reading && !_back_last_state) {
        if (now - _back_last_debounce_time > 100) { // Debounce
            _back_pressed = true;
            _back_last_debounce_time = now;
        }
    }
    _back_last_state = back_reading;

    bool enter_reading = (digitalRead(BTN_ENTER_PIN) == LOW);
    if (enter_reading && !_enter_last_state) {
        if (now - _enter_last_debounce_time > 100) {
            _enter_pressed = true;
            _enter_last_debounce_time = now;
        }
    }
    _enter_last_state = enter_reading;

    bool down_reading = (digitalRead(BTN_DOWN_PIN) == LOW);
    if (down_reading && !_down_last_state) {
        if (now - _down_last_debounce_time > 100) {
            _down_pressed = true;
            _down_last_debounce_time = now;
        }
    }
    _down_last_state = down_reading;

    static long pulse_accumulator = 0;
    noInterrupts();
    int pulses_since_last_update = _encoder_raw_pulses;
    _encoder_raw_pulses = 0;
    interrupts();
    if (pulses_since_last_update != 0) {
        pulse_accumulator += pulses_since_last_update;
        long turn_speed = abs(pulses_since_last_update);
        int pulses_per_step;
        if (turn_speed < EncoderTuning::SPEED_THRESHOLD_MEDIUM) {
            pulses_per_step = EncoderTuning::PULSES_PER_STEP_SLOW;
        } else if (turn_speed < EncoderTuning::SPEED_THRESHOLD_FAST) {
            pulses_per_step = EncoderTuning::PULSES_PER_STEP_MEDIUM;
        } else {
            pulses_per_step = EncoderTuning::PULSES_PER_STEP_FAST;
        }
        if (abs(pulse_accumulator) >= pulses_per_step) {
            _encoder_change = pulse_accumulator / pulses_per_step;
            pulse_accumulator %= pulses_per_step;
        }
    }
}

bool InputManager::wasBackPressed() { return _back_pressed; }
bool InputManager::wasEnterPressed() { return _enter_pressed; }
bool InputManager::wasDownPressed() { return _down_pressed; }
int InputManager::getEncoderChange() { return _encoder_change; }
