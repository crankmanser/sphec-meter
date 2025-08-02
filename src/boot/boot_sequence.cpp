// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

/**
 * @brief --- FIX: The constructor is simplified ---
 */
BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

/**
 * @brief --- FIX: This function is now just an animation runner ---
 * It calls the private animation function and has no other logic.
 */
void BootSelector::runBootAnimation() {
    runAnimation(2000); // Run the animation for a fixed 2 seconds.
}

/**
 * @brief Renders the "walking boot" animation. (Unchanged)
 * @param duration_ms The total duration for the animation to run.
 */
void BootSelector::runAnimation(uint32_t duration_ms) {
    uint32_t start_time = millis();
    Adafruit_SSD1306* display = _displayManager.getDisplay(1);
    if (!display) return;

    while (millis() - start_time < duration_ms) {
        uint32_t elapsed = millis() - start_time;
        int frame_index = (elapsed / 400) % boot_animation_frame_count;
        int x_pos = map(elapsed, 0, duration_ms, -32, SCREEN_WIDTH);
        
        _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
        display->clearDisplay();
        display->drawBitmap(x_pos, 16, boot_animation_frames[frame_index], 32, 32, 1);
        display->display();
        delay(50);
    }
}