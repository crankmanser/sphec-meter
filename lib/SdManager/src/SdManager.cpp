// File Path: /lib/SdManager/src/SdManager.cpp

#include "SdManager.h"
#include <SPI.h>

SdManager::SdManager() : _faultHandler(nullptr), _isInitialized(false), _csPin(0) {}

bool SdManager::begin(FaultHandler& faultHandler, uint8_t csPin) {
    _faultHandler = &faultHandler;
    _csPin = csPin;
    if (!sd.begin(_csPin)) {
        _isInitialized = false;
        return false;
    }
    _isInitialized = true;
    return true;
}

bool SdManager::saveJson(const char* path, const JsonDocument& doc) {
    if (!_isInitialized) return false;
    char tmpPath[256];
    char bakPath[256];
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);
    snprintf(bakPath, sizeof(bakPath), "%s.bak", path);
    
    // Use FsFile, the file type from SdFat's VFS wrapper.
    FsFile tmpFile = sd.open(tmpPath, FILE_WRITE);
    if (!tmpFile) return false;
    if (serializeJson(doc, tmpFile) == 0) {
        tmpFile.close();
        return false;
    }
    tmpFile.close();
    
    if (sd.exists(bakPath)) sd.remove(bakPath);
    
    if (sd.exists(path)) {
        if (!sd.rename(path, bakPath)) {
            sd.remove(tmpPath);
            return false;
        }
    }
    
    if (!sd.rename(tmpPath, path)) {
        sd.rename(bakPath, path);
        return false;
    }
    return true;
}

bool SdManager::loadJson(const char* path, JsonDocument& doc) {
    if (!_isInitialized) return false;
    
    FsFile file = sd.open(path, FILE_READ);
    if (file) {
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error == DeserializationError::Ok) return true;
    }
    
    char bakPath[256];
    snprintf(bakPath, sizeof(bakPath), "%s.bak", path);
    
    FsFile bakFile = sd.open(bakPath, FILE_READ);
    if (bakFile) {
        DeserializationError error = deserializeJson(doc, bakFile);
        bakFile.close();
        if (error == DeserializationError::Ok) return true;
    }
    
    return false;
}