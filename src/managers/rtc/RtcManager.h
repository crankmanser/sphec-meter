// src/managers/rtc/RtcManager.h
#pragma once

#include "hal/PCF8563_Driver.h"
#include "hal/TCA9548_Wrapper.h" // <<< MODIFIED: Use the new wrapper
#include <string>

/**
 * @class RtcManager
 * @brief Manages the Real-Time Clock.
 */
class RtcManager {
public:
    RtcManager(PCF8563_Driver& rtc_driver, TCA9548_Wrapper& tca); // <<< MODIFIED
    void begin(TwoWire* wire);
    void update();

    std::string getDateString();
    std::string getTimeString();
    DateTime now();

private:
    PCF8563_Driver& _rtc_driver;
    TCA9548_Wrapper& _tca; // <<< MODIFIED
    DateTime _cached_time;
    char _date_buffer[11];
    char _time_buffer[9];

    void selectRtcChannel();
};