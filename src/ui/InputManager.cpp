// File Path: /src/ui/InputManager.cpp
// MODIFIED FILE

#include "InputManager.h"
#include <ProjectConfig.h>

// ... (static member initializations are unchanged) ...
volatile long InputManager::_encoder_raw_pulses = 0;
volatile uint8_t InputManager::_last_AB_state = 0;
const int8_t InputManager::_qem_decode_table[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
namespace EncoderTuning {
    const int PULSES_PER_STEP_FINE = 433;
    const int PULSES_PER_STEP_MEDIUM = 161;
    const int PULSES_PER_STEP_FAST = 77;
    const int SPEED_THRESHOLD_SLOW_MAX = 55;
    const int SPEED_THRESHOLD_MEDIUM_MAX = 165;
}
void IRAM_ATTR InputManager::encoderISR() {
    uint8_t pinA = digitalRead(ENCODER_PIN_A);
    uint8_t pinB = digitalRead(ENCODER_PIN_B);
    uint8_t currentState = (pinB << 1) | pinA;
    if (currentState != _last_AB_state) {
        _encoder_raw_pulses += _qem_decode_table[(_last_AB_state << 2) | currentState];
        _last_AB_state = currentState;
    }
}

// ... (constructor and begin() are unchanged) ...
InputManager::InputManager() :
    _accumulated_pulses(0),
    _back_pressed(false), _enter_pressed(false), _down_pressed(false),
    _encoder_change(0),
    _back_last_state(false), _back_last_debounce_time(0),
    _enter_last_state(false), _enter_last_debounce_time(0),
    _down_last_state(false), _down_last_debounce_time(0)
{}

void InputManager::begin() {
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(BTN_BACK_PIN, INPUT_PULLUP);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    uint8_t pinA = digitalRead(ENCODER_PIN_A);
    uint8_t pinB = digitalRead(ENCODER_PIN_B);
    _last_AB_state = (pinB << 1) | pinA;
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);
}

// ... (update() and getters are unchanged) ...
void InputManager::update() {
    noInterrupts();
    long pulsesSinceLastCheck = _encoder_raw_pulses;
    _encoder_raw_pulses = 0;
    interrupts();
    _encoder_change = 0;
    if (pulsesSinceLastCheck != 0) {
        _accumulated_pulses += pulsesSinceLastCheck;
        long turn_speed = abs(pulsesSinceLastCheck);
        int effectivePulsesPerStep;
        if (turn_speed < EncoderTuning::SPEED_THRESHOLD_SLOW_MAX) {
            effectivePulsesPerStep = EncoderTuning::PULSES_PER_STEP_FINE;
        } else if (turn_speed < EncoderTuning::SPEED_THRESHOLD_MEDIUM_MAX) {
            effectivePulsesPerStep = EncoderTuning::PULSES_PER_STEP_MEDIUM;
        } else {
            effectivePulsesPerStep = EncoderTuning::PULSES_PER_STEP_FAST;
        }
        if (abs(_accumulated_pulses) >= effectivePulsesPerStep) {
            _encoder_change = _accumulated_pulses / effectivePulsesPerStep;
            _accumulated_pulses %= effectivePulsesPerStep;
        }
    }
    uint32_t now = millis();
    _back_pressed = false;
    _enter_pressed = false;
    _down_pressed = false;
    bool back_reading = (digitalRead(BTN_BACK_PIN) == LOW);
    if (back_reading && !_back_last_state) { if (now - _back_last_debounce_time > 100) { _back_pressed = true; _back_last_debounce_time = now; } }
    _back_last_state = back_reading;
    bool enter_reading = (digitalRead(BTN_ENTER_PIN) == LOW);
    if (enter_reading && !_enter_last_state) { if (now - _enter_last_debounce_time > 100) { _enter_pressed = true; _enter_last_debounce_time = now; } }
    _enter_last_state = enter_reading;
    bool down_reading = (digitalRead(BTN_DOWN_PIN) == LOW);
    if (down_reading && !_down_last_state) { if (now - _down_last_debounce_time > 100) { _down_pressed = true; _down_last_debounce_time = now; } }
    _down_last_state = down_reading;
}

bool InputManager::wasBackPressed() { return _back_pressed; }
bool InputManager::wasEnterPressed() { return _enter_pressed; }
bool InputManager::wasDownPressed() { return _down_pressed; }
int InputManager::getEncoderChange() { return _encoder_change; }


/**
 * @brief --- NEW: Implementation to clear the enter button's state ---
 * This function resets the debouncer's state for the enter button. It reads
 * the current physical state of the button and synchronizes the internal
 * tracking variables, ensuring that the button release from the boot combo
 * is not misinterpreted as a new press event.
 */
void InputManager::clearEnterButtonState() {
    _enter_last_state = (digitalRead(BTN_ENTER_PIN) == LOW);
    _enter_last_debounce_time = millis();
    _enter_pressed = false;
}