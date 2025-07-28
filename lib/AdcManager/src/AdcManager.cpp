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
{
    _probeState[0] = ProbeState::DORMANT;
    _probeState[1] = ProbeState::DORMANT;
}

bool AdcManager::begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t sdCsPin) {
    _faultHandler = &faultHandler;
    _vspi = spiBus;
    _spiMutex = spiMutex;
    _sdCsPin = sdCsPin;
    _adc1 = new ADS1118(ADC1_CS_PIN, _vspi);
    _adc2 = new ADS1118(ADC2_CS_PIN, _vspi);
    _adc1->begin();
    _adc2->begin();
    _adc1->setSamplingRate(ADS1118::RATE_860SPS);
    _adc2->setSamplingRate(ADS1118::RATE_860SPS);
    _adc1->setFullScaleRange(ADS1118::FSR_4096);
    _adc2->setFullScaleRange(ADS1118::FSR_4096);
    _adc1->setSingleShotMode();
    _adc2->setSingleShotMode();
    _initialized = true;
    return true;
}

void AdcManager::deselectOtherSlaves(uint8_t activeAdcCsPin) {
    digitalWrite(_sdCsPin, HIGH);
    if (activeAdcCsPin == ADC1_CS_PIN) {
        digitalWrite(ADC2_CS_PIN, HIGH);
    } else {
        digitalWrite(ADC1_CS_PIN, HIGH);
    }
}

double AdcManager::getVoltage(uint8_t adcIndex, uint8_t inputs) {
    if (!_initialized || _spiMutex == nullptr || adcIndex > 1) {
        return 0.0;
    }

    if (_probeState[adcIndex] == ProbeState::DORMANT) {
        return 0.0;
    }

    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        ADS1118* adc = (adcIndex == 0) ? _adc1 : _adc2;
        uint8_t currentCsPin = (adcIndex == 0) ? ADC1_CS_PIN : ADC2_CS_PIN;
        
        deselectOtherSlaves(currentCsPin);

        _vspi->beginTransaction(SPISettings(ADS1118::SCLK, MSBFIRST, SPI_MODE1));
        
        // --- FIX: Implement the "Priming Read" as per legacy code ---
        // The first read stabilizes the ADC's internal sampling. Its result is discarded.
        adc->getMilliVolts(inputs); 
        
        // The second read is the actual, stable measurement.
        double voltage = adc->getMilliVolts(inputs);

        _vspi->endTransaction();
        xSemaphoreGive(_spiMutex);

        // Account for 10k+10k voltage divider on probe outputs
        if(inputs == ADS1118::DIFF_0_1) {
            voltage *= 2.0;
        }

        return voltage;
    }
    return 0.0;
}

void AdcManager::setProbeState(uint8_t adcIndex, ProbeState state) {
    if (!_initialized || adcIndex > 1) return;
    ADS1118* adc = (adcIndex == 0) ? _adc1 : _adc2;
    _probeState[adcIndex] = state;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        if (state == ProbeState::ACTIVE) {
            adc->setContinuousMode();
        } else {
            adc->setSingleShotMode();
        }
        xSemaphoreGive(_spiMutex);
    }
}

bool AdcManager::isProbeActive(uint8_t adcIndex) {
    if (adcIndex > 1) return false;
    return _probeState[adcIndex] == ProbeState::ACTIVE;
}