// src/hal/TCA9548_Manual_Driver.h
#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * @class TCA9548_Manual_Driver
 * @brief A simple, direct-control HAL driver for the TCA9548A I2C Multiplexer.
 *
 * This driver avoids third-party libraries and uses direct Wire.h commands,
 * mimicking the stable implementation from the legacy codebase to ensure
 * maximum compatibility and prevent bus conflicts.
 */
class TCA9548_Manual_Driver {
public:
    TCA9548_Manual_Driver(uint8_t address, TwoWire* wire);
    void begin();
    void selectChannel(uint8_t channel);
    void deselectAllChannels();

private:
    uint8_t _address;
    TwoWire* _wire;
};