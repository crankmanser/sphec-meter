// src/hal/PCF8563_Driver.h
#pragma once

#include <RTClib.h>

/**
 * @class PCF8563_Driver
 * @brief A pure HAL driver for the PCF8563 RTC chip.
 *
 * This class wraps the RTClib library to provide a simple, direct
 * interface for interacting with the RTC hardware. It is stateless
 * and only concerned with I2C communication.
 */
class PCF8563_Driver {
public:
    PCF8563_Driver();
    bool begin(TwoWire* wire);

    // Gets the current time from the RTC.
    DateTime now();

    // Sets the RTC to the given time.
    void adjust(const DateTime& dt);

    // Checks if the RTC has been initialized and is running.
    bool isrunning();

private:
    RTC_PCF8563 _rtc;
};