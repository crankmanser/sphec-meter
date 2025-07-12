// src/hal/PCF8563_Driver.cpp
#include "hal/PCF8563_Driver.h"
#include "DebugMacros.h"

PCF8563_Driver::PCF8563_Driver() {}

bool PCF8563_Driver::begin(TwoWire* wire) {
    if (_rtc.begin(wire)) {
        // Check if the RTC is running. If not, it's likely the first boot
        // or the backup battery has failed.
        if (!isrunning()) {
            LOG_MAIN("[RTC_HAL] RTC is NOT running, let's set the time!\n");
            // Set the time to the compile time. The RtcManager will handle this.
            return false;
        }
        return true;
    }
    return false;
}

DateTime PCF8563_Driver::now() {
    return _rtc.now();
}

void PCF8563_Driver::adjust(const DateTime& dt) {
    _rtc.adjust(dt);
}

bool PCF8563_Driver::isrunning() {
    return _rtc.isrunning();
}