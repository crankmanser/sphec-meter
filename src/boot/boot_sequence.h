// File Path: /src/boot/boot_sequence.h
// MODIFIED FILE

#ifndef BOOT_SEQUENCE_H
#define BOOT_SEQUENCE_H

#include "DisplayManager.h"

// This enum is now also used by main.cpp to direct traffic
enum class BootMode {
    NORMAL,
    PBIOS
};

/**
 * @class BootSelector
 * @brief A completely self-contained module for the boot animation and mode selection.
 *
 * This class has no dependencies on the main UI engine (InputManager, etc.).
 * It performs its own simple, direct input polling, making the boot process
 * extremely robust and isolated.
 */
class BootSelector {
public:
    BootSelector(DisplayManager& displayManager);
    BootMode runBootSequence(uint32_t post_duration_ms);

private:
    void drawMenu(BootMode selected_mode);
    void runPostAnimation(uint32_t post_duration_ms);

    DisplayManager& _displayManager;
    
    // Internal state for simple input polling
    int _encoder_last_state;
    long _encoder_pulses;
};

#endif // BOOT_SEQUENCE_H