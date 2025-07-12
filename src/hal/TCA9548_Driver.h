#pragma once
#include <Arduino.h>
#include <Wire.h>

// This is a lightweight, stateless driver for the TCA9548A I2C Multiplexer.
// Its only job is to select or deselect channels on the I2C bus.

class TCA9548_Driver {
public:
    TCA9548_Driver(uint8_t address);
    void begin(TwoWire* wire);

    // Selects a single channel to open for communication.
    bool selectChannel(uint8_t channel);

    // Deselects all channels, isolating all devices on the secondary bus.
    void deselectAllChannels();

private:
    uint8_t _address;
    TwoWire* _wire;
};