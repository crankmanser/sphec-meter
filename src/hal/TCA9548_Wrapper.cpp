// src/hal/TCA9548_Wrapper.cpp
// MODIFIED FILE
#include "TCA9548_Wrapper.h"
#include "DebugMacros.h"

TCA9548_Wrapper::TCA9548_Wrapper(uint8_t address, TwoWire* wire) :
    _tca(address, wire)
{}

bool TCA9548_Wrapper::begin() {
    if (!_tca.begin()) {
        LOG_MAIN("[HAL_ERROR] TCA9548 multiplexer not found at address 0x%X\n", _address);
        return false;
    }
    LOG_HAL("TCA9548 HAL Wrapper initialized for address 0x%X\n", _address);
    return true;
}

bool TCA9548_Wrapper::selectChannel(uint8_t channel) {
    if (channel > 7) {
        LOG_HAL("[TCA_WRAPPER] Error: Invalid channel %d\n", channel);
        return false;
    }
    // <<< FIX: The correct method to select a single channel is selectChannel().
    _tca.selectChannel(channel);
    delay(2);
    return true;
}

void TCA9548_Wrapper::deselectAllChannels() {
    _tca.setChannelMask(0x00);
}

bool TCA9548_Wrapper::isConnected(uint8_t channel) {
    return _tca.isConnected();
}