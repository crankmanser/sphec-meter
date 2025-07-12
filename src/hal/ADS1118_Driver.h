#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "ADS1118.h"
#include "freertos/semphr.h" // <<< ADDED: Include for mutex type

class ADS1118_Driver {
public:
    // <<< MODIFIED: Added SemaphoreHandle_t to the constructor signature
    ADS1118_Driver(uint8_t cs_pin, uint8_t other_adc_cs_pin, uint8_t sd_cs_pin, SPIClass* spi, SemaphoreHandle_t spi_bus_mutex);
    
    void begin();

    int16_t readDifferential_0_1();
    int16_t readSingleEnded_2();
    int16_t readSingleEnded_3();

private:
    SPIClass* _spi;
    ADS1118 _adc;
    uint8_t _cs_pin;
    uint8_t _other_adc_cs_pin;
    uint8_t _sd_cs_pin;
    SemaphoreHandle_t _spi_bus_mutex; // <<< ADDED: Declare the mutex handle

    void deselectOtherSlaves();
};