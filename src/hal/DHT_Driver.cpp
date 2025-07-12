// src/hal/DHT_Driver.cpp
// MODIFIED FILE
#include "DHT_Driver.h"
#include "DebugMacros.h"

DHT_Driver::DHT_Driver(uint8_t pin, uint8_t type) :
    _dht(pin, type),
    _pin(pin),
    _type(type) // <<< ADDED: Initialize our own type variable.
{}

void DHT_Driver::begin() {
    _dht.begin();
    uint8_t detected_type = _dht.read(false) ? 22 : 11;
    // <<< MODIFIED: Use our own '_type' member for logging.
    LOG_HAL("DHT Driver on pin %d initialized. Configured Type: DHT%d, Detected Type: DHT%d\n", _pin, _type, detected_type);
}

float DHT_Driver::getTemperatureC() {
    float tempC = _dht.readTemperature();
    
    if (isnan(tempC)) {
        LOG_HAL("Error: Could not read temperature from DHT sensor on pin %d.\n", _pin);
        return NAN;
    }
    return tempC;
}

float DHT_Driver::getHumidity() {
    float humidity = _dht.readHumidity();
    
    if (isnan(humidity)) {
        LOG_HAL("Error: Could not read humidity from DHT sensor on pin %d.\n", _pin);
        return NAN;
    }
    return humidity;
}