// File Path: /src/boot/boot_sequence.h
// MODIFIED FILE

#ifndef BOOT_SEQUENCE_H
#define BO_SEQUENCE_H

#include "DisplayManager.h"

// The BootMode enum is still used by the main application logic.
enum class BootMode {
    NORMAL,
    PBIOS
};

/**
 * @class BootSelector
 * @brief Manages the boot-up animation display.
 *
 * This class has been simplified to have a single responsibility: rendering the
 * boot-up animation. The complex logic for mode selection has been moved back
 * into main.cpp and now uses a much more reliable, direct hardware read,
 * mirroring the stable legacy implementation. This change resolves critical
 * timing and input bleed-through issues.
 */
class BootSelector {
public:
    // --- FIX: The constructor no longer needs the InputManager ---
    BootSelector(DisplayManager& displayManager);
    
    // --- FIX: This function no longer handles logic, it just shows the animation ---
    void runBootAnimation();

private:
    void runAnimation(uint32_t duration_ms);
    DisplayManager& _displayManager;
};

#endif // BOOT_SEQUENCE_H