#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// This HAL driver encapsulates the complexity of the 1-Wire protocol
// and the DallasTemperature library. Its only job is to provide a
// raw temperature reading.
class DS18B20_Driver {
public:
    // The constructor takes the pin number where the sensor is connected.
    DS18B20_Driver(uint8_t one_wire_pin);

    void begin();

    // Requests a new temperature reading from the sensor.
    void requestTemperatures();

    // Returns the last requested temperature in Celsius.
    float getTempC();

private:
    uint8_t _pin;
    
    // Instances of the underlying libraries.
    OneWire _one_wire;
    DallasTemperature _sensors;
};