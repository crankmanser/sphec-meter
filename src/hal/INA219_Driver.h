#pragma once
#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <Wire.h>

// This HAL driver provides a simple, stateless interface to the INA219
// power monitor. It wraps the Adafruit library to conform to our
// project's architectural principles.

class INA219_Driver {
public:
    INA219_Driver(uint8_t address);

    // Initializes the underlying library. Must be called in setup().
    bool begin(TwoWire* wire);

    // Reads the bus voltage (the main battery voltage). Returns volts.
    float getBusVoltage();

    // Reads the shunt voltage (voltage across the sense resistor). Returns millivolts.
    float getShuntVoltage_mV();

    // Reads the calculated current draw of the system. Returns milliamps.
    float getCurrent_mA();

private:
    Adafruit_INA219 _ina219;
    uint8_t _address;
    bool _initialized;
};