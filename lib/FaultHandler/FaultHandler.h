// File Path: /lib/FaultHandler/FaultHandler.h

#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#include <Arduino.h>

/**
 * @class FaultHandler
 * @brief A centralized cabinet for managing and responding to system-wide faults.
 *
 * This class provides a unified interface for logging errors, tracking system state,
 * and triggering appropriate responses to critical failures. It is designed to be
 * a singleton or a globally accessible object to ensure a consistent error handling
 * strategy across the entire firmware.
 */
class FaultHandler {
public:
    /**
     * @brief Constructor for the FaultHandler.
     */
    FaultHandler();

    /**
     * @brief Initializes the FaultHandler.
     * This method should be called once during the `setup()` function.
     * @param initial_serial_baud The baud rate for the serial monitor.
     */
    void begin(long initial_serial_baud = 115200);

    /**
     * @brief Triggers a critical system fault.
     * This method is designed to handle unrecoverable errors. It will print a
     * detailed fault message to the serial port and then halt execution in an
     * infinite loop to prevent further damage or unexpected behavior.
     *
     * @param fault_code A unique code identifying the fault.
     * @param message A descriptive message explaining the reason for the fault.
     * @param file The file in which the fault occurred (automatically captured).
     * @param line The line number at which the fault occurred (automatically captured).
     */
    void trigger_fault(const char* fault_code, const char* message, const char* file, int line);

private:
    bool _initialized; // Tracks if the handler has been initialized.
};

#endif // FAULT_HANDLER_H