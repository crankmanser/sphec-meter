// File Path: /lib/INA219_Driver/src/INA219_Driver.h

#ifndef INA219_DRIVER_H
#define INA219_DRIVER_H

#include <Adafruit_INA219.h>
#include <Wire.h>
#include "ProjectConfig.h" // For I2C address
#include <FaultHandler.h>

/**
 * @class INA219_Driver
 * @brief A Hardware Abstraction Layer (HAL) cabinet for the INA219 Power Monitor.
 *
 * This cabinet wraps the Adafruit_INA219 library to provide a simple, standardized
 * interface for the rest of the firmware. It is responsible for direct, stateless
 * communication with the hardware.
 */
class INA219_Driver {
public:
    /**
     * @brief Constructor for the INA219_Driver.
     */
    INA219_Driver();

    /**
     * @brief Initializes the INA219 sensor.
     * @param faultHandler A reference to the global fault handler for error reporting.
     * @return True if initialization is successful, false otherwise.
     */
    virtual bool begin(FaultHandler& faultHandler);

    /**
     * @brief Gets the bus voltage from the sensor.
     * @return The bus voltage in Volts.
     */
    virtual float getBusVoltage();

    /**
     * @brief Gets the current from the sensor.
     * @return The current in Milliamps.
     */
    virtual float getCurrent_mA();

private:
    FaultHandler* _faultHandler;
    Adafruit_INA219 _ina219;
    bool _initialized;
};

#endif // INA219_DRIVER_H