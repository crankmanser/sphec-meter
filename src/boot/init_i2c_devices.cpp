// src/boot/init_i2c_devices.cpp
// MODIFIED FILE
#include "init_i2c_devices.h"
#include <Wire.h>
#include "config/hardware_config.h"
#include "DebugMacros.h"
#include "hal/INA219_Driver.h"
#include "presentation/DisplayManager.h"
#include "managers/rtc/RtcManager.h"
#include "hal/TCA9548_Manual_Driver.h"


// External declarations for the global objects this function will initialize.
extern TwoWire i2c;
extern INA219_Driver* ina219;
extern DisplayManager* displayManager;
extern RtcManager* rtcManager;
extern TCA9548_Manual_Driver* tca9548;

bool init_i2c_devices() {
    LOG_MAIN("--- Initializing I2C Devices ---\n");

    i2c.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    LOG_MAIN("I2C Bus physical layer started.");

    // Initialize the I2C Multiplexer first.
    if(tca9548) {
        tca9548->begin();
    } else {
        LOG_MAIN("[I2C_INIT] CRITICAL: TCA9548 Mux object is null!");
        return false;
    }

    // Initialize devices in the known stable order.

    // 1. INA219 (Power Monitor) - Directly on the bus
    if (!ina219 || !ina219->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] WARNING: INA219 Power Monitor failed to initialize!");
        // This is a warning, not a critical failure for boot.
    } else {
        LOG_MAIN("INA219 Power Monitor Initialized.");
    }

    // 2. DisplayManager (OLEDs) - Behind the multiplexer
    if (!displayManager || !displayManager->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] CRITICAL: DisplayManager failed to initialize!");
        return false; // This is critical for UI.
    } else {
        LOG_MAIN("DisplayManager Initialized.");
    }

    // 3. RtcManager (RTC) - Behind the multiplexer
    if (!rtcManager || !rtcManager->begin(&i2c)) {
        LOG_MAIN("[I2C_INIT] WARNING: RtcManager failed to initialize or set time.");
        // This is a warning, not a critical failure for boot.
    } else {
        LOG_MAIN("RtcManager Initialized.");
    }

    LOG_MAIN("--- I2C Device Initialization Complete ---\n");
    return true;
}