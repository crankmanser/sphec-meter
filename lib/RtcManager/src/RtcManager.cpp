// File Path: /lib/RtcManager/src/RtcManager.cpp
// MODIFIED FILE

#include "RtcManager.h"
#include "ProjectConfig.h" // For TCA_CHANNEL

/**
 * @brief Constructor for the RtcManager.
 * Initializes the member variables.
 */
RtcManager::RtcManager() :
    _faultHandler(nullptr),
    _initialized(false),
    _i2cMutex(nullptr)
{}

/**
 * @brief Initializes the RTC hardware.
 * Takes the I2C mutex, selects the correct TCA channel, and begins
 * communication with the RTC chip.
 *
 * @param faultHandler A reference to the global fault handler.
 * @param i2cMutex A handle to the mutex protecting the I2C bus.
 * @return True if initialization is successful, false otherwise.
 */
bool RtcManager::begin(FaultHandler& faultHandler, SemaphoreHandle_t i2cMutex) {
    _faultHandler = &faultHandler;
    _i2cMutex = i2cMutex;

    if (xSemaphoreTake(_i2cMutex, portMAX_DELAY) == pdTRUE) {
        // Select the RTC on the I2C multiplexer
        Wire.beginTransmission(TCA_ADDRESS);
        Wire.write(1 << RTC_TCA_CHANNEL);
        Wire.endTransmission();

        // --- DEFINITIVE FIX: Use the return value of begin() to check for success ---
        if (!_rtc.begin()) {
            _initialized = false;
        } else {
            // Check if the RTC has lost power since it was last set.
             _initialized = !_rtc.lostPower();
        }

        xSemaphoreGive(_i2cMutex);
    } else {
        _initialized = false;
    }
    
    // If the RTC has lost power, set it to a default time based on compile time.
    if (_initialized && _rtc.lostPower()) {
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    return _initialized;
}

/**
 * @brief Gets the current time and formats it as a string.
 * The format is YYYYMMDD-HHMMSS, suitable for filenames.
 *
 * @param buffer The character buffer to store the timestamp string.
 * @param bufferSize The size of the buffer.
 */
void RtcManager::getTimestamp(char* buffer, size_t bufferSize) {
    if (!_initialized || buffer == nullptr) {
        snprintf(buffer, bufferSize, "00000000-000000");
        return;
    }

    DateTime now;
    if (xSemaphoreTake(_i2cMutex, portMAX_DELAY) == pdTRUE) {
        now = _rtc.now();
        xSemaphoreGive(_i2cMutex);
    }
    
    snprintf(buffer, bufferSize, "%04d%02d%02d-%02d%02d%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());
}

/**
 * @brief Checks if the RTC is running.
 * @return True if the RTC was initialized successfully.
 */
bool RtcManager::isRunning() {
    return _initialized;
}