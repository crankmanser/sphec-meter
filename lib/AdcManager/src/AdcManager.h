// File Path: /lib/AdcManager/src/AdcManager.h
// MODIFIED FILE

#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include <SPI.h>
#include <ADS1118.h>
#include "ProjectConfig.h"
#include <FaultHandler.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

enum class ProbeState {
    DORMANT,
    ACTIVE
};

class AdcManager {
public:
    AdcManager();
    bool begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t sdCsPin);
    
    double getVoltage(uint8_t adcIndex, uint8_t inputs);

    /**
     * @brief --- NEW: A non-blocking version of getVoltage. ---
     * This function performs a raw ADC reading without taking the SPI mutex.
     * It is intended for use in functions that already have exclusive control
     * of the SPI bus, preventing deadlocks.
     */
    double getVoltage_noLock(uint8_t adcIndex, uint8_t inputs);

    void setProbeState(uint8_t adcIndex, ProbeState state);
    bool isProbeActive(uint8_t adcIndex);


private:
    void deselectOtherSlaves(uint8_t activeAdcCsPin);

    FaultHandler* _faultHandler;
    bool _initialized;
    SPIClass* _vspi;
    ADS1118* _adc1;
    ADS1118* _adc2;
    SemaphoreHandle_t _spiMutex;
    uint8_t _sdCsPin;
    ProbeState _probeState[2];
};

#endif // ADC_MANAGER_H