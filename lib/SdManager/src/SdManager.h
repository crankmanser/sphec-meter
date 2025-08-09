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
    bool mkdir(const char* path);
    FsFile open(const char* path, oflag_t oflag);
    bool remove(const char* path);

    /**
     * @brief --- NEW: Takes exclusive control of the SPI bus mutex. ---
     * This should be called before a sequence of long file operations.
     * @return True if the mutex was successfully taken.
     */
    bool takeMutex();

    /**
     * @brief --- NEW: Releases the SPI bus mutex. ---
     * This must be called after a sequence of long file operations is complete.
     */
    void giveMutex();

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