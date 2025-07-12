// src/managers/io/EncoderManager.cpp
#include "managers/io/EncoderManager.h"

// Define static members
volatile int_fast16_t EncoderManager::_position = 0;
volatile uint8_t EncoderManager::_state = 0;

EncoderManager::EncoderManager(uint8_t pin_a, uint8_t pin_b) :
    _pin_a(pin_a),
    _pin_b(pin_b)
{}

void IRAM_ATTR EncoderManager::update_isr() {
    uint8_t pin_a_val = digitalRead(25); // Hardcoded for ISR performance
    uint8_t pin_b_val = digitalRead(26);
    uint8_t s = _state & 3;
    if (pin_a_val) s |= 4;
    if (pin_b_val) s |= 8;
    _state = (s >> 2);

    switch (s) {
        case 1: case 7: case 8: case 14:
            _position++;
            return;
        case 2: case 4: case 11: case 13:
            _position--;
            return;
        case 3: case 12:
            _position += 2;
            return;
        case 6: case 9:
            _position -= 2;
            return;
    }
}

void EncoderManager::begin() {
    pinMode(_pin_a, INPUT_PULLUP);
    pinMode(_pin_b, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin_a), update_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pin_b), update_isr, CHANGE);
}

int EncoderManager::getChange() {
    int_fast16_t current_pos = _position;
    int_fast16_t diff = current_pos - _last_position;
    _last_position = current_pos;

    uint32_t now = millis();
    uint32_t time_diff = now - _last_read_time;
    _last_read_time = now;

    // "Speed Engine" logic from legacy project
    if (time_diff > 0 && abs(diff) > 0) {
        int pulses_per_ms = abs(diff) * 1000 / time_diff;
        if (pulses_per_ms > 300) { // Fast turn
            _pulses_per_step = 1;
        } else if (pulses_per_ms > 100) { // Medium turn
            _pulses_per_step = 2;
        } else { // Slow turn
            _pulses_per_step = 4;
        }
    }
    
    return diff / _pulses_per_step;
}