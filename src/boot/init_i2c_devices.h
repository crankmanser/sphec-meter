// src/boot/init_i2c_devices.h
#pragma once

/**
 * @brief Initializes all I2C devices in the correct sequence.
 *
 * This function centralizes the initialization of all I2C-based hardware
 * to ensure the bus is stable and devices are brought up in the exact
 * order that is proven to work in the legacy firmware.
 *
 * @return true if all devices initialize successfully, false otherwise.
 */
bool init_i2c_devices();