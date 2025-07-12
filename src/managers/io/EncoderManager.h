// src/managers/io/EncoderManager.h
#pragma once

#include <Arduino.h>

/**
 * @class EncoderManager
 * @brief Manages the rotary encoder using interrupts and a "Speed Engine".
 *
 * This is a refactor of the legacy EncoderManager, which provided a "silky smooth"
 * user experience by implementing speed-sensitive acceleration for scrolling.
 */
class EncoderManager {
public:
    EncoderManager(uint8_t pin_a, uint8_t pin_b);
    void begin();
    int getChange(); // Returns the number of logical UI steps since the last call.

private:
    const uint8_t _pin_a;
    const uint8_t _pin_b;

    // Volatile as they are modified in an ISR.
    static volatile int_fast16_t _position;
    static volatile uint8_t _state;

    // For the "Speed Engine"
    int_fast16_t _last_position = 0;
    uint32_t _last_read_time = 0;
    int _pulses_per_step = 4; // Start with high precision

    static void IRAM_ATTR update_isr(); // Interrupt Service Routine
};