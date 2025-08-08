// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

/**
 * @brief --- DEFINITIVE FIX: The constructor is simplified ---
 * The boot selector's only responsibility is to show the animation.
 * All boot logic is now handled directly and robustly in main.cpp.
 */
BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

/**
 * @brief --- DEFINITIVE FIX: This function is now just an animation runner ---
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

    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    display->clearDisplay();
    display->setTextSize(2);
    display->setCursor(10, 20);
    display->print("SpHEC Meter");
    display->setTextSize(1);
    display->setCursor(35, 40);
    display->print("v3.1.1");
    display->display();
    
    while (millis() - start_time < duration_ms) {
        // Simple delay to show the static screen
        delay(50);
    }
}