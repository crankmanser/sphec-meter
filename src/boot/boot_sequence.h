// File Path: /src/boot/boot_sequence.h
// MODIFIED FILE

#ifndef BOOT_SEQUENCE_H
#define BOOT_SEQUENCE_H

#include "DisplayManager.h"

enum class BootMode {
    NORMAL,
    PBIOS
};

class BootSelector {
public:
    BootSelector(DisplayManager& displayManager);

    /**
     * @brief Runs the entire unified boot sequence.
     * This single public method handles both the POST animation and the
     * interactive boot selection menu.
     * @param post_duration_ms The time to run the POST animation.
     * @return The selected BootMode.
     */
    BootMode runBootSequence(uint32_t post_duration_ms);

private:
    void drawMenu(BootMode selected_mode);
    void runPostAnimation(uint32_t post_duration_ms);

    DisplayManager& _displayManager;
};

#endif // BOOT_SEQUENCE_H