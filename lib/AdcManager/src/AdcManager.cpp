// File Path: /lib/AdcManager/src/AdcManager.cpp
// MODIFIED FILE

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

// This is the standard, thread-safe public method.
double AdcManager::getVoltage(uint8_t adcIndex, uint8_t inputs) {
    if (!_initialized || _spiMutex == nullptr) return 0.0;

    double voltage = 0.0;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        voltage = getVoltage_noLock(adcIndex, inputs);
        xSemaphoreGive(_spiMutex);
    }
    return voltage;
}

/**
 * @brief --- NEW: Implementation of the non-locking getVoltage method. ---
 * This function contains the core ADC reading logic and is now called by both
 * the standard getVoltage and the GuidedTuningEngine.
 */
double AdcManager::getVoltage_noLock(uint8_t adcIndex, uint8_t inputs) {
    if (!_initialized || adcIndex > 1) return 0.0;
    if (_probeState[adcIndex] == ProbeState::DORMANT) return 0.0;

    ADS1118* adc = (adcIndex == 0) ? _adc1 : _adc2;
    uint8_t currentCsPin = (adcIndex == 0) ? ADC1_CS_PIN : ADC2_CS_PIN;
    
    deselectOtherSlaves(currentCsPin);

    _vspi->beginTransaction(SPISettings(ADS1118::SCLK, MSBFIRST, SPI_MODE1));
    
    // The "Priming Read" is critical for stable readings.
    adc->getMilliVolts(inputs); 
    double voltage = adc->getMilliVolts(inputs);

    _vspi->endTransaction();

    // Account for the voltage divider on the probe inputs.
    if(inputs == ADS1118::DIFF_0_1) {
        voltage *= 2.0;
    }

    return voltage;
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