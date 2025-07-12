#include "LDR_Driver.h"
#include "DebugMacros.h"

// The LDR circuit is read with a Full-Scale Range of 4.096V
const float ADC_FSR_LDR = 4.096; 

LDR_Driver::LDR_Driver(ADS1118_Driver* adc) : _adc(adc) {
    LOG_HAL("LDR Driver initialized.\n");
}

float LDR_Driver::getVoltage() {
    // Ensure the ADC pointer is valid before using it.
    if (!_adc) {
        LOG_HAL("LDR Driver Error: ADC driver is null.\n");
        return 0.0f;
    }

    // Use the new method on the ADS1118_Driver to get the raw 16-bit value.
    int16_t raw_value = _adc->readSingleEnded_3();

    // Convert the raw integer value to a voltage.
    float voltage = (static_cast<float>(raw_value) / 32767.0) * ADC_FSR_LDR;

    return voltage;
}