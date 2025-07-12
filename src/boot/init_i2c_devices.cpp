// src/boot/init_i2c_devices.cpp
// MODIFIED FILE
#include "init_i2c_devices.h"
#include <Wire.h>
#include "config/hardware_config.h"
#include "DebugMacros.h"
#include "presentation/DisplayManager.h"
#include "managers/rtc/RtcManager.h"
#include "hal/INA219_Driver.h"

// External declarations for the global objects this function will initialize.
extern TwoWire i2c;
extern DisplayManager* displayManager;
extern RtcManager* rtcManager;
extern INA219_Driver* ina219;

bool init_i2c_devices() {
    LOG_MAIN("--- Initializing I2C Devices ---\n");

    i2c.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    LOG_MAIN("I2C Bus physical layer started.");

    if (!ina219 || !ina219->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] CRITICAL: INA219 Power Monitor failed to initialize!");
    } else {
        LOG_MAIN("INA219 Power Monitor Initialized.");
    }

    if (!displayManager || !displayManager->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] CRITICAL: DisplayManager failed to initialize!");
        return false;
    } else {
        LOG_MAIN("DisplayManager Initialized.");
    }

    // <<< FIX: This check is now valid because RtcManager::begin() returns a bool.
    if (!rtcManager || !rtcManager->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] WARNING: RtcManager failed to initialize or set time.");
    } else {
        LOG_MAIN("RtcManager Initialized.");
    }

    LOG_MAIN("--- I2C Device Initialization Complete ---\n");
    return true;
}