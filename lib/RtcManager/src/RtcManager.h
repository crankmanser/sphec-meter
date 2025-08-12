// File Path: /lib/RtcManager/src/RtcManager.h
// NEW FILE

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <RTClib.h>
#include <FaultHandler.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @class RtcManager
 * @brief A cabinet for managing the PCF8563 Real-Time Clock.
 *
 * This class encapsulates all interactions with the RTC hardware, providing
 * thread-safe methods to initialize the clock and retrieve the current time.
 * It is designed to be initialized once at boot to prevent race conditions.
 */
class RtcManager {
public:
    /**
     * @brief Constructor for the RtcManager.
     */
    RtcManager();

    /**
     * @brief Initializes the RTC hardware.
     * @param faultHandler A reference to the global fault handler.
     * @param i2cMutex A handle to the mutex protecting the I2C bus.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(FaultHandler& faultHandler, SemaphoreHandle_t i2cMutex);

    /**
     * @brief Gets the current time from the RTC as a formatted string.
     * @param buffer The character buffer to store the timestamp string.
     * @param bufferSize The size of the buffer.
     */
    void getTimestamp(char* buffer, size_t bufferSize);

    /**
     * @brief Checks if the RTC is running and the time is valid.
     * @return True if the RTC is running, false otherwise.
     */
    bool isRunning();

private:
    FaultHandler* _faultHandler;
    RTC_PCF8563 _rtc;
    bool _initialized;
    SemaphoreHandle_t _i2cMutex;
};

#endif // RTC_MANAGER_H