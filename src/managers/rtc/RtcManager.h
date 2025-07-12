// src/managers/rtc/RtcManager.h
#pragma once

#include "hal/PCF8563_Driver.h"
#include "hal/TCA9548_Driver.h"
#include <string>

/**
 * @class RtcManager
 * @brief Manages the Real-Time Clock.
 *
 * This cabinet is responsible for all timekeeping functions. It handles
 * I2C multiplexer selection and provides formatted date and time strings
 * for the rest of the application.
 */
class RtcManager {
public:
    RtcManager(PCF8563_Driver& rtc_driver, TCA9548_Driver& tca);
    void begin(TwoWire* wire);
    void update(); // Periodically updates the cached time

    std::string getDateString();
    std::string getTimeString();
    DateTime now();

private:
    PCF8563_Driver& _rtc_driver;
    TCA9548_Driver& _tca;
    DateTime _cached_time;
    char _date_buffer[11]; // "YYYY-MM-DD"
    char _time_buffer[9];  // "HH:MM:SS"

    void selectRtcChannel();
};