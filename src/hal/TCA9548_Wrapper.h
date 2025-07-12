// src/hal/TCA9548_Wrapper.h
#pragma once

#include <Arduino.h>
#include <TCA9548.h> // The new, stable library from Rob Tillaart

/**
 * @class TCA9548_Wrapper
 * @brief A pure HAL "Cabinet" that wraps the RobTillaart/TCA9548 library.
 *
 * This class provides a stable, consistent interface to the I2C multiplexer,
 * adhering to the project's layered architecture while leveraging a robust,
 * professional-grade third-party library to ensure bus stability.
 */
class TCA9548_Wrapper {
public:
    TCA9548_Wrapper(uint8_t address, TwoWire* wire);
    bool begin();
    bool selectChannel(uint8_t channel);
    void deselectAllChannels();
    bool isConnected(uint8_t channel);

private:
    TCA9548 _tca;
    uint8_t _address;
    TwoWire* _wire;
};