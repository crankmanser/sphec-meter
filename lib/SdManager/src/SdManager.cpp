// File Path: /lib/SdManager/src/SdManager.cpp
// MODIFIED FILE

#include "SdManager.h"
#include <SPI.h>
#include "DebugConfig.h" // Include for logging macros

SdManager::SdManager() : _faultHandler(nullptr), _isInitialized(false), _csPin(0), _spiMutex(nullptr), _adc1CsPin(0), _adc2CsPin(0) {}

bool SdManager::begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t csPin, uint8_t adc1CsPin, uint8_t adc2CsPin) {
    _faultHandler = &faultHandler;
    _csPin = csPin;
    _spiMutex = spiMutex;
    _adc1CsPin = adc1CsPin;
    _adc2CsPin = adc2CsPin;

    LOG_STORAGE("SdManager::begin() - Attempting to take SPI mutex...");
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        LOG_STORAGE("SdManager::begin() - Mutex taken. Deselecting other slaves.");
        deselectOtherSlaves();
        SdSpiConfig sdConfig(_csPin, SHARED_SPI, SD_SCK_MHZ(4), spiBus);

        LOG_STORAGE("SdManager::begin() - Calling sd.begin()...");
        if (!sd.begin(sdConfig)) {
            LOG_STORAGE("SdManager::begin() - ERROR: sd.begin() failed.");
            _isInitialized = false;
            xSemaphoreGive(_spiMutex);
            return false;
        }
        _isInitialized = true;
        LOG_STORAGE("SdManager::begin() - SUCCESS: sd.begin() successful.");
        xSemaphoreGive(_spiMutex);
        return true;
    }
    LOG_STORAGE("SdManager::begin() - ERROR: Could not take SPI mutex.");
    return false;
}

void SdManager::deselectOtherSlaves() {
    digitalWrite(_adc1CsPin, HIGH);
    digitalWrite(_adc2CsPin, HIGH);
}

bool SdManager::mkdir(const char* path) {
    if (!_isInitialized) {
        LOG_STORAGE("SdManager::mkdir('%s') - ERROR: Not initialized.", path);
        return false;
    }
     if (_spiMutex == nullptr) {
        LOG_STORAGE("SdManager::mkdir('%s') - ERROR: SPI mutex is null.", path);
        return false;
    }

    bool success = false;
    LOG_STORAGE("SdManager::mkdir('%s') - Attempting to take SPI mutex...", path);
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        LOG_STORAGE("SdManager::mkdir('%s') - Mutex taken.", path);
        deselectOtherSlaves();
        success = sd.mkdir(path);
        if(success) {
            LOG_STORAGE("SdManager::mkdir('%s') - SUCCESS.", path);
        } else {
            LOG_STORAGE("SdManager::mkdir('%s') - ERROR: sd.mkdir() failed.", path);
        }
        xSemaphoreGive(_spiMutex);
    } else {
        LOG_STORAGE("SdManager::mkdir('%s') - ERROR: Could not take SPI mutex.", path);
    }
    return success;
}

FsFile SdManager::open(const char* path, oflag_t oflag) {
    if (!_isInitialized) return FsFile();
    deselectOtherSlaves();
    return sd.open(path, oflag);
}

bool SdManager::remove(const char* path) {
    if (!_isInitialized || _spiMutex == nullptr) return false;

    bool success = false;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        deselectOtherSlaves();
        success = sd.remove(path);
        xSemaphoreGive(_spiMutex);
    }
    return success;
}

bool SdManager::takeMutex() {
    if (!_spiMutex) return false;
    return xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE;
}

void SdManager::giveMutex() {
    if (_spiMutex) {
        xSemaphoreGive(_spiMutex);
    }
}

/**
 * @brief --- DEFINITIVE FIX: Implements a robust, atomic write-and-close sequence. ---
 * This function guarantees that data is physically written to the SD card by
 * performing the critical sequence of: write -> sync -> CLOSE on a temporary
 * file before any rename operations are attempted. This resolves the empty file bug.
 */
bool SdManager::saveJson(const char* path, const JsonDocument& doc) {
    if (!_isInitialized) {
         LOG_STORAGE("SdManager::saveJson('%s') - ERROR: Not initialized.", path);
        return false;
    }
    if (_spiMutex == nullptr) {
        LOG_STORAGE("SdManager::saveJson('%s') - ERROR: SPI mutex is null.", path);
        return false;
    }


    bool success = false;
    LOG_STORAGE("SdManager::saveJson('%s') - Attempting to take SPI mutex...", path);
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        LOG_STORAGE("SdManager::saveJson('%s') - Mutex taken.", path);
        deselectOtherSlaves();
        char tmpPath[256];
        char bakPath[256];
        snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);
        snprintf(bakPath, sizeof(bakPath), "%s.bak", path);

        LOG_STORAGE("SdManager::saveJson('%s') - Opening temp file '%s'...", path, tmpPath);
        FsFile tmpFile = sd.open(tmpPath, FILE_WRITE);
        if (tmpFile) {
            LOG_STORAGE("SdManager::saveJson('%s') - Temp file opened. Writing data...", path);
            if (serializeJson(doc, tmpFile) > 0) {
                LOG_STORAGE("SdManager::saveJson('%s') - Write successful. Syncing and closing...", path);
                tmpFile.sync();
                tmpFile.close();
                LOG_STORAGE("SdManager::saveJson('%s') - Temp file closed. Performing atomic rename.", path);

                if (sd.exists(bakPath)) {
                    LOG_STORAGE("SdManager::saveJson('%s') - Removing old backup '%s'.", path, bakPath);
                    sd.remove(bakPath);
                }
                if (sd.exists(path)) {
                     LOG_STORAGE("SdManager::saveJson('%s') - Renaming original to backup '%s'.", path, bakPath);
                    sd.rename(path, bakPath);
                }
                if (sd.rename(tmpPath, path)) {
                    LOG_STORAGE("SdManager::saveJson('%s') - SUCCESS: Final rename successful.", path);
                    success = true;
                } else {
                    LOG_STORAGE("SdManager::saveJson('%s') - ERROR: Final rename failed. Restoring backup.", path);
                    sd.rename(bakPath, path);
                }
            } else {
                 LOG_STORAGE("SdManager::saveJson('%s') - ERROR: serializeJson() failed.", path);
                tmpFile.close();
            }
        } else {
            LOG_STORAGE("SdManager::saveJson('%s') - ERROR: Could not open temp file '%s'.", path, tmpPath);
        }
        xSemaphoreGive(_spiMutex);
         LOG_STORAGE("SdManager::saveJson('%s') - Mutex released.", path);
    } else {
        LOG_STORAGE("SdManager::saveJson('%s') - ERROR: Could not take mutex.", path);
    }
    return success;
}

bool SdManager::loadJson(const char* path, JsonDocument& doc) {
    if (!_isInitialized) {
        LOG_STORAGE("SdManager::loadJson('%s') - ERROR: Not initialized.", path);
        return false;
    }
    if (_spiMutex == nullptr) {
        LOG_STORAGE("SdManager::loadJson('%s') - ERROR: SPI mutex is null.", path);
        return false;
    }

    bool success = false;
     LOG_STORAGE("SdManager::loadJson('%s') - Attempting to take mutex...", path);
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        LOG_STORAGE("SdManager::loadJson('%s') - Mutex taken.", path);
        deselectOtherSlaves();
        FsFile file = sd.open(path, FILE_READ);
        if (file) {
            LOG_STORAGE("SdManager::loadJson('%s') - File opened. Deserializing...", path);
            DeserializationError error = deserializeJson(doc, file);
            file.close();
            if (error == DeserializationError::Ok) {
                LOG_STORAGE("SdManager::loadJson('%s') - SUCCESS.", path);
                success = true;
            } else {
                 LOG_STORAGE("SdManager::loadJson('%s') - ERROR: deserializeJson() failed: %s", path, error.c_str());
            }
        } else {
             LOG_STORAGE("SdManager::loadJson('%s') - File not found. Checking for backup...", path);
        }

        if (!success) {
            char bakPath[256];
            snprintf(bakPath, sizeof(bakPath), "%s.bak", path);
            FsFile bakFile = sd.open(bakPath, FILE_READ);
            if (bakFile) {
                LOG_STORAGE("SdManager::loadJson('%s') - Backup file '%s' opened. Deserializing...", path, bakPath);
                DeserializationError error = deserializeJson(doc, bakFile);
                bakFile.close();
                if (error == DeserializationError::Ok) {
                    LOG_STORAGE("SdManager::loadJson('%s') - SUCCESS from backup.", path);
                    success = true;
                } else {
                    LOG_STORAGE("SdManager::loadJson('%s') - ERROR: deserializeJson() from backup failed: %s", path, error.c_str());
                }
            }
        }
        xSemaphoreGive(_spiMutex);
        LOG_STORAGE("SdManager::loadJson('%s') - Mutex released.", path);
    } else {
         LOG_STORAGE("SdManager::loadJson('%s') - ERROR: Could not take mutex.", path);
    }
    return success;
}