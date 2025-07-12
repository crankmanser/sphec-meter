// src/managers/rtc/RtcManager.h
// MODIFIED FILE
#pragma once

#include "hal/PCF8563_Driver.h"
#include "hal/TCA9548_Manual_Driver.h" // <<< MODIFIED
#include <string>

class RtcManager {
public:
    RtcManager(PCF8563_Driver& rtc_driver, TCA9548_Manual_Driver& tca); // <<< MODIFIED
    void begin(TwoWire* wire);
    void update();

    std::string getDateString();
    std::string getTimeString();
    DateTime now();

private:
    PCF8563_Driver& _rtc_driver;
    TCA9548_Manual_Driver& _tca; // <<< MODIFIED
    DateTime _cached_time;
    char _date_buffer[11];
    char _time_buffer[9];

    void selectRtcChannel();
};