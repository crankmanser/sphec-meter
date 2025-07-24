// File Path: /lib/FaultHandler/FaultHandler.cpp

#include "FaultHandler.h"

//
// --- CONSTRUCTOR ---
//
FaultHandler::FaultHandler() : _initialized(false) {
    // Constructor is intentionally left empty.
    // Initialization is handled in the begin() method to control the timing.
}

//
// --- INITIALIZATION ---
//
void FaultHandler::begin(long initial_serial_baud) {
    // If no active Serial connection, start one.
    if (!Serial) {
        Serial.begin(initial_serial_baud);
        // Wait for the serial port to connect. Needed for native USB port only.
        while (!Serial) {
            ;
        }
    }
    _initialized = true;
}

//
// --- FAULT TRIGGER ---
//
void FaultHandler::trigger_fault(const char* fault_code, const char* message, const char* file, int line) {
    // Ensure the serial monitor is running to display the error.
    if (!_initialized) {
        begin();
    }

    // Print a detailed, formatted error message to the Serial monitor.
    Serial.println("========================================");
    Serial.println("!!!       CRITICAL SYSTEM FAULT      !!!");
    Serial.println("========================================");
    Serial.printf("  - Fault Code: %s\n", fault_code);
    Serial.printf("  - Message:    %s\n", message);
    Serial.printf("  - Location:   %s, Line %d\n", file, line);
    Serial.println("----------------------------------------");
    Serial.println("The system has been halted to prevent further issues.");
    Serial.println("Please review the fault details above and restart the device.");
    Serial.println("========================================");

    // Halt the system. In a real-time OS, this is safer than letting a
    // critically faulted task continue.
    while (1) {
        delay(1000); // Prevent watchdog timer resets on some platforms.
    }
}