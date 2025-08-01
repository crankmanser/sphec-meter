// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

// The RTC variable is no longer needed in this file as the logic is
// now centralized in main.cpp.

/**
 * @brief Constructor for the BootSelector.
 * @param displayManager A reference to the global DisplayManager.
 */
BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager)
{
    // The constructor is kept simple as this class no longer manages input state.
}

/**
 * @brief Runs the boot sequence.
 * @details In the current stable architecture, this function's sole
 * responsibility is to display the boot-up animation and then return,
 * allowing the main setup() function to proceed with the boot logic.
 */
void BootSelector::runBootSequence() {
    runPostAnimation(2000); // Display the animation for 2 seconds.
}

/**
 * @brief Renders the "walking boot" animation on the middle OLED.
 * @param post_duration_ms The total duration for the animation to run.
 */
void BootSelector::runPostAnimation(uint32_t post_duration_ms) {
    uint32_t start_time = millis();
    // --- FIX: Corrected the typo from Adafruit_SSD1_1306 to Adafruit_SSD1306 ---
    Adafruit_SSD1306* display = _displayManager.getDisplay(1); // Get middle display
    if (!display) return;

    while (millis() - start_time < post_duration_ms) {
        uint32_t elapsed = millis() - start_time;
        
        int frame_index = (elapsed / 400) % boot_animation_frame_count;
        int x_pos = map(elapsed, 0, post_duration_ms, -32, SCREEN_WIDTH);
        
        _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
        display->clearDisplay();
        display->drawBitmap(x_pos, 16, boot_animation_frames[frame_index], 32, 32, 1);
        display->display();
        
        delay(50);
    }
}