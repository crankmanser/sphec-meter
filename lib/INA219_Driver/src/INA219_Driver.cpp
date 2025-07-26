// File Path: /lib/INA219_Driver/src/INA219_Driver.cpp

#include "INA219_Driver.h"

/**
 * @brief Constructor for the INA219_Driver.
 *
 * Initializes the underlying Adafruit_INA219 object with the I2C address
 * defined in ProjectConfig.h. This ensures that all hardware-specific
 * constants are centralized in one configuration file.
 */
INA219_Driver::INA219_Driver() : 
    _faultHandler(nullptr), 
    _ina219(INA219_I2C_ADDRESS), 
    _initialized(false) 
{}

/**
 * @brief Initializes the INA219 sensor.
 *
 * This method calls the begin() function of the underlying Adafruit library
 * to initialize the I2C communication with the sensor. It also performs a
 * simple validation check to see if it can get a plausible reading, which
 * helps confirm that the sensor is connected and functioning.
 *
 * @param faultHandler A reference to the global fault handler for error reporting.
 * @return True if initialization is successful, false otherwise.
 */
bool INA219_Driver::begin(FaultHandler& faultHandler) {
    _faultHandler = &faultHandler;
    
    // The Adafruit library's begin() method initializes the I2C communication.
    _ina219.begin();

    // Perform a simple check to see if the device is responsive.
    // If the bus voltage is greater than 0, it's a good sign that the
    // sensor is connected and providing data.
    if (_ina219.getBusVoltage_V() > 0) {
        _initialized = true;
        return true;
    } else {
        // If the check fails, we do not set the initialized flag.
        // This prevents other methods from trying to read from a non-existent sensor.
        _initialized = false;
        return false;
    }
}

/**
 * @brief Gets the bus voltage from the sensor.
 *
 * This method reads the main voltage of the power source (i.e., the battery).
 * It includes a safety check to ensure the driver has been successfully
 * initialized before attempting to read from the sensor.
 *
 * @return The bus voltage in Volts, or 0.0 if not initialized.
 */
float INA219_Driver::getBusVoltage() {
    if (!_initialized) {
        return 0.0f;
    }
    return _ina219.getBusVoltage_V();
}

/**
 * @brief Gets the current from the sensor.
 *
 * This method reads the current being consumed by the system. The value can
 * be positive (charging) or negative (discharging). It includes the same
 * safety check as getBusVoltage().
 *
 * @return The current in Milliamps, or 0.0 if not initialized.
 */
float INA219_Driver::getCurrent_mA() {
    if (!_initialized) {
        return 0.0f;
    }
    return _ina219.getCurrent_mA();
}