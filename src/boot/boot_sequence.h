// File Path: /src/boot/boot_sequence.h
// MODIFIED FILE

#ifndef BOOT_SEQUENCE_H
#define BOOT_SEQUENCE_H

#include "DisplayManager.h"

// The BootMode enum is still useful for the main logic
enum class BootMode {
    NORMAL,
    PBIOS
};

// The RTC variable is no longer needed.

class BootSelector {
public:
    BootSelector(DisplayManager& displayManager);
    // The function is simplified, it no longer returns a value.
    void runBootSequence();

private:
    // The menu drawing is no longer needed.
    void runPostAnimation(uint32_t post_duration_ms);
    DisplayManager& _displayManager;
};

#endif // BOOT_SEQUENCE_H