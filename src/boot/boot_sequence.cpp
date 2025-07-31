// File Path: /src/boot/boot_sequence.cpp
// MODIFIED FILE

#include "boot_sequence.h"
#include "ProjectConfig.h"
#include "boot_animation.h"

// --- CRITICAL FIX: Define the RTC variable with the correct attribute ---
// and WITHOUT an initial value.
RTC_NOINIT_ATTR BootMode rtc_boot_mode;

BootSelector::BootSelector(DisplayManager& displayManager) :
    _displayManager(displayManager),
    _encoder_last_state(0),
    _encoder_pulses(0)
{}

/**
 * @brief Runs the boot selection UI.
 *
 * This function now blocks until the user makes a selection. It then saves
 * that selection to RTC memory and triggers a software reboot.
 * @param post_duration_ms Duration of the initial boot animation.
 */


void BootSelector::runBootSequence(uint32_t post_duration_ms) {
    runPostAnimation(post_duration_ms);

    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);

    BootMode selected_mode = BootMode::NORMAL;
    
    while (true) {
        // --- FIX: Reverted to the original, simple, and reliable polling logic ---
        int current_state = digitalRead(ENCODER_PIN_A);
        if (current_state != _encoder_last_state) {
            // A simple rising-edge pulse count is sufficient and robust for this pre-RTOS context.
            if (digitalRead(ENCODER_PIN_B) != current_state) {
                _encoder_pulses++;
            } else {
                _encoder_pulses--;
            }
        }
        _encoder_last_state = current_state;

        // Use a simple threshold. A value of 2 provides a responsive feel.
        if (abs(_encoder_pulses) >= 2) {
            selected_mode = (selected_mode == BootMode::NORMAL) ? BootMode::PBIOS : BootMode::NORMAL;
            _encoder_pulses = 0;
        }

        if (digitalRead(BTN_ENTER_PIN) == LOW) {
            delay(50); // Debounce
            if (digitalRead(BTN_ENTER_PIN) == LOW) {
                printf("[BOOT SELECTOR] Saving boot mode choice: %d\n", (int)selected_mode);
                rtc_boot_mode = selected_mode;
                printf("[BOOT SELECTOR] Rebooting now...\n");
                ESP.restart();
            }
        }
        
        drawMenu(selected_mode);
        delay(10);
    }
}

// ... runPostAnimation and drawMenu methods are unchanged ...
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

    const char* prompt = "Select";
    int16_t x1, y1;
    uint16_t w, h;
    display_middle->getTextBounds(prompt, 0, 0, &x1, &y1, &w, &h);
    display_middle->setCursor((SCREEN_WIDTH - w) / 2, SCREEN_HEIGHT - h - 2);
    display_middle->setTextColor(1);
    display_middle->print(prompt);

    display_middle->display();

    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if(display_top) { display_top->clearDisplay(); display_top->display(); }
    
    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if(display_bottom) { display_bottom->clearDisplay(); display_bottom->display(); }
}