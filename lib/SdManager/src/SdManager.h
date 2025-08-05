// File Path: /lib/SdManager/src/SdManager.h
// MODIFIED FILE

#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "I_StorageProvider.h"
#include <FaultHandler.h>
#include <SdFat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class SdManager : public I_StorageProvider {
public:
    SdManager();
    bool begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t csPin, uint8_t adc1CsPin, uint8_t adc2CsPin);
    bool saveJson(const char* path, const JsonDocument& doc) override;
    bool loadJson(const char* path, JsonDocument& doc) override;

    /**
     * @brief Creates a new directory on the SD card.
     * @param path The absolute path of the directory to create.
     * @return True if the directory was created successfully or already exists.
     */
    bool mkdir(const char* path);

private:
    void deselectOtherSlaves();

    FaultHandler* _faultHandler;
    bool _isInitialized;
    uint8_t _csPin;
    SdFat sd;
    SemaphoreHandle_t _spiMutex;
    uint8_t _adc1CsPin;
    uint8_t _adc2CsPin;
};

#endif // SD_MANAGER_H