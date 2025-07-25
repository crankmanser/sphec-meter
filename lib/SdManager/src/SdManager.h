// File Path: /lib/SdManager/src/SdManager.h

#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "I_StorageProvider.h" // Include the local interface header
#include <FaultHandler.h>
#include <SdFat.h>

class SdManager : public I_StorageProvider {
public:
    SdManager();
    bool begin(FaultHandler& faultHandler, uint8_t csPin);
    bool saveJson(const char* path, const JsonDocument& doc) override;
    bool loadJson(const char* path, JsonDocument& doc) override;

private:
    FaultHandler* _faultHandler;
    bool _isInitialized;
    uint8_t _csPin;
    SdFat sd;
};

#endif // SD_MANAGER_H