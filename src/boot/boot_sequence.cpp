// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager),
    _encoder_last_state(0),
    _encoder_pulses(0)
{}

BootMode BootSelector::runBootSequence(uint32_t post_duration_ms) {
    runPostAnimation(post_duration_ms);

    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);

    BootMode selected_mode = BootMode::NORMAL;
    
    while (true) {
        // --- Simple Encoder Polling ---
        int enc_a = digitalRead(ENCODER_PIN_A);
        int enc_b = digitalRead(ENCODER_PIN_B);
        int current_state = (enc_b << 1) | enc_a;
        if(current_state != _encoder_last_state) {
            _encoder_pulses++;
        }
        _encoder_last_state = current_state;

        if (_encoder_pulses >= 4) {
            selected_mode = (selected_mode == BootMode::NORMAL) ? BootMode::PBIOS : BootMode::NORMAL;
            _encoder_pulses = 0;
        }

        // --- Simple Button Polling ---
        if (digitalRead(BTN_ENTER_PIN) == LOW) {
            delay(50);
            if (digitalRead(BTN_ENTER_PIN) == LOW) {
                return selected_mode;
            }
        }
        
        drawMenu(selected_mode);
        delay(10);
    }
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

    // Draw Menu Items
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

    // --- FIX: Draw the button prompt directly ---
    const char* prompt = "Select";
    int16_t x1, y1;
    uint16_t w, h;
    display_middle->getTextBounds(prompt, 0, 0, &x1, &y1, &w, &h);
    display_middle->setCursor((SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT - h - 2);
    display_middle->setTextColor(1);
    display_middle->print(prompt);

    display_middle->display();

    // Clear other screens
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if(display_top) { display_top->clearDisplay(); display_top->display(); }
    
    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if(display_bottom) { display_bottom->clearDisplay(); display_bottom->display(); }
}
