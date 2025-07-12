#include "DS18B20_Driver.h"
#include "DebugMacros.h"

DS18B20_Driver::DS18B20_Driver(uint8_t one_wire_pin) : 
    _pin(one_wire_pin),
    _one_wire(_pin),
    _sensors(&_one_wire)
{}

void DS18B20_Driver::begin() {
    _sensors.begin();
    LOG_HAL("DS18B20 Driver on pin %d initialized.\n", _pin);
}

void DS18B20_Driver::requestTemperatures() {
    _sensors.requestTemperatures(); 
}

float DS18B20_Driver::getTempC() {
    // We use getTempCByIndex(0) because we only have one sensor on the bus.
    float tempC = _sensors.getTempCByIndex(0);
    
    // The library returns a specific error value if the read fails.
    if(tempC == DEVICE_DISCONNECTED_C) {
        LOG_HAL("Error: Could not read temperature from DS18B20.\n");
        return NAN; // Return Not-a-Number for error handling upstream.
    }
    
    return tempC;
}