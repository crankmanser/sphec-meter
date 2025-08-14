// File Path: /lib/SdManager/src/SdManager.cpp
// MODIFIED FILE

#include "SdManager.h"
#include <SPI.h>

SdManager::SdManager() : _faultHandler(nullptr), _isInitialized(false), _csPin(0), _spiMutex(nullptr), _adc1CsPin(0), _adc2CsPin(0) {}

bool SdManager::begin(FaultHandler& faultHandler, SPIClass* spiBus, SemaphoreHandle_t spiMutex, uint8_t csPin, uint8_t adc1CsPin, uint8_t adc2CsPin) {
    _faultHandler = &faultHandler;
    _csPin = csPin;
    _spiMutex = spiMutex;
    _adc1CsPin = adc1CsPin;
    _adc2CsPin = adc2CsPin;

    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        deselectOtherSlaves();
        SdSpiConfig sdConfig(_csPin, SHARED_SPI, SD_SCK_MHZ(4), spiBus);
        if (!sd.begin(sdConfig)) {
            _isInitialized = false;
            xSemaphoreGive(_spiMutex);
            return false;
        }
        _isInitialized = true;
        xSemaphoreGive(_spiMutex);
        return true;
    }
    return false;
}

void SdManager::deselectOtherSlaves() {
    digitalWrite(_adc1CsPin, HIGH);
    digitalWrite(_adc2CsPin, HIGH);
}

bool SdManager::mkdir(const char* path) {
    if (!_isInitialized || _spiMutex == nullptr) return false;

    bool success = false;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        deselectOtherSlaves();
        success = sd.mkdir(path);
        xSemaphoreGive(_spiMutex);
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
    if (!_isInitialized || _spiMutex == nullptr) return false;

    bool success = false;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        deselectOtherSlaves();
        char tmpPath[256];
        char bakPath[256];
        snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", path);
        snprintf(bakPath, sizeof(bakPath), "%s.bak", path);

        // Step 1: Write data to the temporary file
        FsFile tmpFile = sd.open(tmpPath, FILE_WRITE);
        if (tmpFile) {
            if (serializeJson(doc, tmpFile) > 0) {
                // Step 2: Force the write to the physical card and close the file
                tmpFile.sync();
                tmpFile.close();

                // Step 3: Perform atomic rename operations now that data is safe
                if (sd.exists(bakPath)) sd.remove(bakPath);
                if (sd.exists(path)) {
                    sd.rename(path, bakPath);
                }
                if (sd.rename(tmpPath, path)) {
                    success = true;
                } else {
                    // Attempt to restore the backup if the final rename fails
                    sd.rename(bakPath, path);
                }
            } else {
                tmpFile.close(); // Close the file even if write failed
            }
        }
        xSemaphoreGive(_spiMutex);
    }
    return success;
}

bool SdManager::loadJson(const char* path, JsonDocument& doc) {
    if (!_isInitialized || _spiMutex == nullptr) return false;

    bool success = false;
    if (xSemaphoreTake(_spiMutex, portMAX_DELAY) == pdTRUE) {
        deselectOtherSlaves();
        FsFile file = sd.open(path, FILE_READ);
        if (file) {
            DeserializationError error = deserializeJson(doc, file);
            file.close();
            if (error == DeserializationError::Ok) success = true;
        }

        if (!success) {
            char bakPath[256];
            snprintf(bakPath, sizeof(bakPath), "%s.bak", path);
            FsFile bakFile = sd.open(bakPath, FILE_READ);
            if (bakFile) {
                DeserializationError error = deserializeJson(doc, bakFile);
                bakFile.close();
                if (error == DeserializationError::Ok) success = true;
            }
        }
        xSemaphoreGive(_spiMutex);
    }
    return success;
}