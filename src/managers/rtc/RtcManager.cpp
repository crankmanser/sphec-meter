// src/managers/rtc/RtcManager.cpp
#include "managers/rtc/RtcManager.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

RtcManager::RtcManager(PCF8563_Driver& rtc_driver, TCA9548_Driver& tca) :
    _rtc_driver(rtc_driver),
    _tca(tca)
{}

void RtcManager::begin(TwoWire* wire) {
    LOG_MANAGER("Initializing RtcManager...\n");
    selectRtcChannel();
    if (!_rtc_driver.begin(wire)) {
        // If begin() returns false, it means the RTC is not running.
        // We set it to the compile time as a one-time setup.
        LOG_MANAGER("Setting RTC to compile time.\n");
        _rtc_driver.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    update(); // Get the initial time
}

void RtcManager::update() {
    selectRtcChannel();
    _cached_time = _rtc_driver.now();

    // Format the date and time into our buffers
    snprintf(_date_buffer, sizeof(_date_buffer), "%04d-%02d-%02d", 
             _cached_time.year(), _cached_time.month(), _cached_time.day());
    snprintf(_time_buffer, sizeof(_time_buffer), "%02d:%02d:%02d", 
             _cached_time.hour(), _cached_time.minute(), _cached_time.second());
}

std::string RtcManager::getDateString() {
    return std::string(_date_buffer);
}

std::string RtcManager::getTimeString() {
    return std::string(_time_buffer);
}

DateTime RtcManager::now() {
    selectRtcChannel();
    return _rtc_driver.now();
}

void RtcManager::selectRtcChannel() {
    _tca.selectChannel(RTC_TCA_CHANNEL);
}