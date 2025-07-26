// File Path: /lib/AdcManager/src/AdcManager.cpp

#include "AdcManager.h"

AdcManager::AdcManager() :
    _faultHandler(nullptr),
    _initialized(false),
    _vspi(nullptr),
    _adc1(nullptr),
    _adc2(nullptr),
    _spiMutex(nullptr),
    _sdCsPin(0)
{}

// --- FIX: Store the SD card CS pin ---
bool AdcManager::begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t sdCsPin) {
    _faultHandler = &faultHandler;
    _vspi = spiBus;
    _spiMutex = spiMutex;
    _sdCsPin = sdCsPin; // Store SD CS pin

    _adc1 = new ADS1118(ADC1_CS_PIN, _vspi);
    _adc2 = new ADS1118(ADC2_CS_PIN, _vspi);

    _adc1->begin();
    _adc2->begin();

    _adc1->setSamplingRate(ADS1118::RATE_860SPS);
    _adc2->setSamplingRate(ADS1118::RATE_860SPS);
    _adc1->setFullScaleRange(ADS1118::FSR_4096);
    _adc2->setFullScaleRange(ADS1118::FSR_4096);

    _initialized = true;
    return true;
}

// --- FIX: New method for explicit bus arbitration ---
void AdcManager::deselectOtherSlaves(uint8_t activeAdcCsPin) {
    // De-select the SD card
    digitalWrite(_sdCsPin, HIGH);
    // De-select the *other* ADC
    if (activeAdcCsPin == ADC1_CS_PIN) {
        digitalWrite(ADC2_CS_PIN, HIGH);
    } else {
        digitalWrite(ADC1_CS_PIN, HIGH);
    }
}

double AdcManager::getVoltage(uint8_t adcIndex) {
    if (!_initialized || _spiMutex == nullptr) {
        return 0.0;
    }

    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        uint8_t currentCsPin = (adcIndex == 0) ? ADC1_CS_PIN : ADC2_CS_PIN;
        
        // --- FIX: Perform bus arbitration before the transaction ---
        deselectOtherSlaves(currentCsPin);

        _vspi->beginTransaction(SPISettings(ADS1118::SCLK, MSBFIRST, SPI_MODE1));
        
        double voltage = 0.0;
        if (adcIndex == 0) {
            voltage = _adc1->getMilliVolts();
        } else if (adcIndex == 1) {
            voltage = _adc2->getMilliVolts();
        }

        _vspi->endTransaction();
        xSemaphoreGive(_spiMutex);
        return voltage;
    }
    return 0.0;
}