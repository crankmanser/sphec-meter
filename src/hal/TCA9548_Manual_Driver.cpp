// src/hal/TCA9548_Manual_Driver.cpp
#include "TCA9548_Manual_Driver.h"
#include "DebugMacros.h"

TCA9548_Manual_Driver::TCA9548_Manual_Driver(uint8_t address, TwoWire* wire) :
    _address(address),
    _wire(wire)
{}

void TCA9548_Manual_Driver::begin() {
    // No complex begin, just ensure the wire object is valid.
    LOG_HAL("TCA9548 Manual Driver initialized for address 0x%X\n", _address);
}

void TCA9548_Manual_Driver::selectChannel(uint8_t channel) {
    if (channel > 7) return;
    if (!_wire) return;

    _wire->beginTransmission(_address);
    _wire->write(1 << channel);
    if (_wire->endTransmission() != 0) {
        LOG_MAIN("[TCA_MANUAL] Error selecting channel %d\n", channel);
    }
    delay(2); // Allow bus to settle
}

void TCA9548_Manual_Driver::deselectAllChannels() {
    if (!_wire) return;
    _wire->beginTransmission(_address);
    _wire->write(0x00);
    _wire->endTransmission();
}