#include "DHT_Driver.h"
#include "DebugMacros.h"

DHT_Driver::DHT_Driver(uint8_t pin, uint8_t type) :
    _dht(pin, type),
    _pin(pin) // <<< ADDED: Initialize our own pin variable
{}

void DHT_Driver::begin() {
    _dht.begin();
    // <<< MODIFIED: Use our own _pin variable, not one from the DHT library object.
    LOG_HAL("DHT%d Driver on pin %d initialized.\n", _dht.read(false) ? 22 : 11, _pin);
}

float DHT_Driver::getTemperatureC() {
    float tempC = _dht.readTemperature();
    
    if (isnan(tempC)) {
        LOG_HAL("Error: Could not read temperature from DHT sensor.\n");
        return NAN;
    }
    return tempC;
}

float DHT_Driver::getHumidity() {
    float humidity = _dht.readHumidity();
    
    if (isnan(humidity)) {
        LOG_HAL("Error: Could not read humidity from DHT sensor.\n");
        return NAN;
    }
    return humidity;
}