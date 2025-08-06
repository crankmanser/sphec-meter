// File Path: /lib/TempManager/src/TempManager.h
// MODIFIED FILE

#ifndef TEMP_MANAGER_H
#define TEMP_MANAGER_H

#include <Arduino.h>
#include <FaultHandler.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include "ProjectConfig.h"

class TempManager {
public:
    TempManager();
    bool begin(FaultHandler& faultHandler);
    void update();
    
    /**
     * @brief Initiates a temperature conversion on the DS18B20 sensor.
     * This is a non-blocking call; the result must be read later via update().
     */
    void requestProbeTemperature();

    float getProbeTemp();
    float getAmbientTemp();
    float getHumidity();

    void setProbeTempOffset(float offset);
    void setAmbientTempOffset(float offset);

private:
    FaultHandler* _faultHandler;
    bool _initialized;

    OneWire _oneWire;
    DallasTemperature _dallasSensors;
    DHT _dht;

    float _probeTempC;
    float _ambientTempC;
    float _humidity;

    float _probeTempOffset;
    float _ambientTempOffset;
};

#endif // TEMP_MANAGER_H