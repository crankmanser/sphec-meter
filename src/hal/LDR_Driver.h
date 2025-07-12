#pragma once
#include "hal/ADS1118_Driver.h"

// This HAL driver reads the LDR light sensor by using the ADS1118_Driver.
// Its only job is to return the raw voltage from the sensor circuit.
// Converting this voltage to a meaningful unit like Lux or a percentage
// is the responsibility of a Manager-level cabinet.

class LDR_Driver {
public:
    // The LDR is on ADC1, so we pass a pointer to it during construction.
    LDR_Driver(ADS1118_Driver* adc);

    // Reads and returns the raw voltage from the LDR circuit.
    float getVoltage();

private:
    ADS1118_Driver* _adc; // A pointer to the ADC driver it depends on
};