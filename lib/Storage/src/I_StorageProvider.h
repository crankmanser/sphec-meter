// File Path: /lib/SdManager/src/I_StorageProvider.h

#ifndef I_STORAGE_PROVIDER_H
#define I_STORAGE_PROVIDER_H

#include <ArduinoJson.h>

class I_StorageProvider {
public:
    virtual ~I_StorageProvider() {}
    virtual bool saveJson(const char* path, const JsonDocument& doc) = 0;
    virtual bool loadJson(const char* path, JsonDocument& doc) = 0;
};

#endif // I_STORAGE_PROVIDER_H