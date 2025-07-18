// src/boot/init_i2c_devices.cpp
// MODIFIED FILE
#include "init_i2c_devices.h"
#include "app/AppContext.h"
#include <Wire.h>
#include "config/hardware_config.h"
#include "DebugMacros.h"

// Include the full definitions for the managers/drivers being used
#include "hal/INA219_Driver.h"
#include "presentation/DisplayManager.h"
#include "managers/rtc/RtcManager.h"
#include "hal/TCA9548_Manual_Driver.h"

bool init_i2c_devices(AppContext* appContext) {
    LOG_MAIN("--- Initializing I2C Devices ---\n");

    // <<< THE FINAL FIX: Initialize the physical I2C bus immediately before use >>>
    appContext->i2c->begin(I2C_SDA_PIN, I2C_SCL_PIN);
    LOG_MAIN("I2C Bus physical layer started.");

    // Now, initialize the drivers that use the bus.
    appContext->tca->begin();

    // Initialize devices in the known-stable order.
    if (!appContext->displayManager || !appContext->displayManager->begin(appContext->i2c)) {
        LOG_MAIN("[I2C_INIT] CRITICAL: DisplayManager failed to initialize!");
        return false;
    } else {
        LOG_MAIN("DisplayManager Initialized.");
    }

    if (!appContext->rtcManager || !appContext->rtcManager->begin(appContext->i2c)) {
        LOG_MAIN("[I2C_INIT] WARNING: RtcManager failed to initialize or set time.");
    } else {
        LOG_MAIN("RtcManager Initialized.");
    }
    
    if (!appContext->ina219 || !appContext->ina219->begin(appContext->i2c)) {
        LOG_MAIN("[I2C_INIT] WARNING: INA219 Power Monitor failed to initialize!");
    } else {
        LOG_MAIN("INA219 Power Monitor Initialized.");
    }

    LOG_MAIN("--- I2C Device Initialization Complete ---\n");
    return true;
}