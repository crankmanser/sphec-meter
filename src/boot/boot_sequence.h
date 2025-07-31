// File Path: /src/boot/boot_sequence.h
// MODIFIED FILE

#ifndef BOOT_SEQUENCE_H
#define BOOT_SEQUENCE_H

#include "DisplayManager.h"

enum class BootMode {
    NORMAL,
    PBIOS
};

// --- CRITICAL FIX: Use RTC_NOINIT_ATTR instead of RTC_DATA_ATTR ---
// This attribute places the variable in a section of RTC memory that is
// explicitly NOT initialized by the bootloader. This is the correct way
// to ensure a value survives a software reboot via ESP.restart().
extern RTC_NOINIT_ATTR BootMode rtc_boot_mode;

class BootSelector {
public:
    BootSelector(DisplayManager& displayManager);
    void runBootSequence(uint32_t post_duration_ms);

private:
    void drawMenu(BootMode selected_mode);
    void runPostAnimation(uint32_t post_duration_ms);

    DisplayManager& _displayManager;
    
    int _encoder_last_state;
    long _encoder_pulses;
};

#endif // BOOT_SEQUENCE_H