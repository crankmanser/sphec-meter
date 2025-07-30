// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

// This is now the single, public entry point for the boot UI.
BootMode BootSelector::runBootSequence(uint32_t post_duration_ms) {
    runPostAnimation(post_duration_ms);

    BootMode selected_mode = BootMode::NORMAL;
    const uint32_t timeout_ms = 5000;
    uint32_t start_time = millis();

    while (millis() - start_time < timeout_ms) {
        if (digitalRead(BTN_BACK_PIN) == HIGH) {
            selected_mode = BootMode::PBIOS;
            drawMenu(selected_mode);
            delay(500);
            return selected_mode;
        }
        if (digitalRead(BTN_DOWN_PIN) == HIGH) {
            selected_mode = BootMode::NORMAL;
            drawMenu(selected_mode);
            delay(500);
            return selected_mode;
        }
        
        drawMenu(selected_mode);
        delay(50);
    }

    return BootMode::NORMAL;
}

void BootSelector::runPostAnimation(uint32_t post_duration_ms) {
    uint32_t start_time = millis();
    Adafruit_SSD1306* display = _displayManager.getDisplay(1);
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

void BootSelector::drawMenu(BootMode selected_mode) {
    Adafruit_SSD1306* display_middle = _displayManager.getDisplay(1);
    if (!display_middle) return;
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    display_middle->clearDisplay();
    display_middle->setTextSize(1);
    display_middle->setFont(nullptr);
    const char* item1 = "pBios";
    const char* item2 = "StartUp";
    if (selected_mode == BootMode::PBIOS) {
        display_middle->fillRect(0, 20, 128, 12, 1);
        display_middle->setTextColor(0);
    } else {
        display_middle->setTextColor(1);
    }
    display_middle->setCursor(4, 22);
    display_middle->print(item1);
    if (selected_mode == BootMode::NORMAL) {
        display_middle->fillRect(0, 32, 128, 12, 1);
        display_middle->setTextColor(0);
    } else {
        display_middle->setTextColor(1);
    }
    display_middle->setCursor(4, 34);
    display_middle->print(item2);
    display_middle->display();
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if(display_top) {
        display_top->clearDisplay();
        display_top->display();
    }
    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if(display_bottom) {
        display_bottom->clearDisplay();
        display_bottom->display();
    }
}