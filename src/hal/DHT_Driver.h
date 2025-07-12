#pragma once

#include <Arduino.h>
#include <DHT.h>

// This HAL driver encapsulates the Adafruit DHT library to provide
// a simple interface for reading ambient temperature and humidity.
class DHT_Driver {
public:
    // The constructor takes the pin number and the specific DHT type (e.g., DHT11).
    DHT_Driver(uint8_t pin, uint8_t type);

    void begin();

    // Returns the ambient temperature in Celsius.
    float getTemperatureC();

    // Returns the relative humidity as a percentage.
    float getHumidity();

private:
    // Instance of the underlying Adafruit library.
    DHT _dht;
    
    // <<< ADDED: The missing member variable to store the pin number.
    uint8_t _pin; 
};