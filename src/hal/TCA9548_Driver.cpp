#include "TCA9548_Driver.h"
#include "DebugMacros.h"

TCA9548_Driver::TCA9548_Driver(uint8_t address) : _address(address), _wire(nullptr) {}

void TCA9548_Driver::begin(TwoWire* wire) {
    _wire = wire;
}

bool TCA9548_Driver::selectChannel(uint8_t channel) {
    if (channel > 7) {
        LOG_HAL("TCA9548 Error: Invalid channel %d requested.\n", channel);
        return false;
    }
    if (!_wire) {
        LOG_HAL("TCA9548 Error: Wire bus not initialized.\n");
        return false;
    }

    _wire->beginTransmission(_address);
    _wire->write(1 << channel);
    if (_wire->endTransmission() != 0) {
        LOG_HAL("TCA9548 Error: Failed to select channel %d.\n", channel);
        return false;
    }
    
    // A small delay to allow the bus to settle after a channel switch.
    delay(2);
    return true;
}

void TCA9548_Driver::deselectAllChannels() {
    if (!_wire) {
        LOG_HAL("TCA9548 Error: Wire bus not initialized.\n");
        return;
    }
    _wire->beginTransmission(_address);
    _wire->write(0x00);
    _wire->endTransmission();
}