#include "INA219_Driver.h"
#include "DebugMacros.h"

INA219_Driver::INA219_Driver(uint8_t address) :
    _ina219(address),
    _address(address),
    _initialized(false)
{}

bool INA219_Driver::begin(TwoWire* wire) {
    // The Adafruit library uses the global Wire instance by default,
    // but we pass our specific I2C instance for consistency.
    _ina219.begin(wire);

    // Check if the device is actually connected and responding.
    if (_ina219.getBusVoltage_V() > 0) { // A simple check to see if we get a reading
        _initialized = true;
        LOG_HAL("INA219 Driver on address 0x%X initialized.\n", _address);
        return true;
    } else {
        LOG_HAL("INA219 Driver on address 0x%X failed to initialize.\n", _address);
        return false;
    }
}

float INA219_Driver::getBusVoltage() {
    if (!_initialized) return 0.0f;
    return _ina219.getBusVoltage_V();
}

float INA219_Driver::getShuntVoltage_mV() {
    if (!_initialized) return 0.0f;
    return _ina219.getShuntVoltage_mV();
}

float INA219_Driver::getCurrent_mA() {
    if (!_initialized) return 0.0f;
    return _ina219.getCurrent_mA();
}