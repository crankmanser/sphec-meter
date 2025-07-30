// File Path: /lib/INA219_Driver/src/INA219_Driver.cpp
// MODIFIED FILE

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
    _initialized(false),
    _i2cMutex(nullptr)
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
 * @param i2cMutex A handle to the mutex protecting the I2C bus.
 * @return True if initialization is successful, false otherwise.
 */
bool INA219_Driver::begin(FaultHandler& faultHandler, SemaphoreHandle_t i2cMutex) {
    _faultHandler = &faultHandler;
    _i2cMutex = i2cMutex;
    
    // The Adafruit library's begin() method initializes the I2C communication.
    // We wrap it in our mutex to ensure thread safety.
    if (xSemaphoreTake(_i2cMutex, portMAX_DELAY) == pdTRUE) {
        _ina219.begin();
        xSemaphoreGive(_i2cMutex);
    }

    // Perform a simple check to see if the device is responsive.
    if (getBusVoltage() > 0) {
        _initialized = true;
        return true;
    } else {
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
    if (!_initialized || _i2cMutex == nullptr) {
        return 0.0f;
    }
    float voltage = 0.0f;
    if (xSemaphoreTake(_i2cMutex, portMAX_DELAY) == pdTRUE) {
        voltage = _ina219.getBusVoltage_V();
        xSemaphoreGive(_i2cMutex);
    }
    return voltage;
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
    if (!_initialized || _i2cMutex == nullptr) {
        return 0.0f;
    }
    float current = 0.0f;
    if (xSemaphoreTake(_i2cMutex, portMAX_DELAY) == pdTRUE) {
        current = _ina219.getCurrent_mA();
        xSemaphoreGive(_i2cMutex);
    }
    return current;
}