// File Path: /lib/TempManager/src/TempManager.h

#ifndef TEMP_MANAGER_H
#define TEMP_MANAGER_H

#include <Arduino.h>
#include <FaultHandler.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include "ProjectConfig.h"

/**
 * @class TempManager
 * @brief Manages all temperature sensors for the device.
 *
 * This cabinet initializes and reads from the DS18B20 (for liquid temperature)
 * and the DHT11 (for ambient air temperature). It also stores and applies
 * simple calibration offsets for each sensor to ensure accuracy against a
 * trusted reference.
 */
class TempManager {
public:
    /**
     * @brief Constructor for the TempManager.
     */
    TempManager();

    /**
     * @brief Initializes the TempManager and underlying sensors.
     * @param faultHandler A reference to the global fault handler.
     * @return True if initialization is successful.
     */
    bool begin(FaultHandler& faultHandler);

    /**
     * @brief Reads all temperature sensors and updates internal values.
     * This should be called periodically by a dedicated task.
     */
    void update();

    // --- Public Getters ---
    float getProbeTemp();   // Gets the calibrated liquid temperature (DS18B20)
    float getAmbientTemp(); // Gets the calibrated ambient temperature (DHT11)
    float getHumidity();    // Gets the humidity from the DHT11

    // --- Calibration Methods ---
    void setProbeTempOffset(float offset);
    void setAmbientTempOffset(float offset);

private:
    FaultHandler* _faultHandler;
    bool _initialized;

    // --- Sensor Objects ---
    OneWire _oneWire;
    DallasTemperature _dallasSensors;
    DHT _dht;

    // --- Live Sensor Readings (Calibrated) ---
    float _probeTempC;
    float _ambientTempC;
    float _humidity;

    // --- Calibration Offsets ---
    float _probeTempOffset;
    float _ambientTempOffset;
};

#endif // TEMP_MANAGER_H