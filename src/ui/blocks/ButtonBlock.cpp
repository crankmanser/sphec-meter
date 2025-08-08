// File Path: /src/ui/blocks/ButtonBlock.cpp
// MODIFIED FILE

#include "ButtonBlock.h"
#include <Adafruit_GFX.h>

void ButtonBlock::draw(DisplayManager& displayManager, const ButtonBlockProps& props) {
    auto draw_single_prompt = [&](uint8_t display_index, uint8_t tca_channel, const std::string& text) {
        if (text.empty()) return;

        Adafruit_SSD1306* display = displayManager.getDisplay(display_index);
        if (!display) return;

        displayManager.selectTCAChannel(tca_channel);
        
        int16_t x1, y1;
        uint16_t w, h;
        display->setTextSize(1);
        display->setFont(nullptr);
        display->getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

        // --- DEFINITIVE FIX: Move the button up by 1 pixel for more clearance ---
        int cursor_x = (SCREEN_WIDTH - w) / 2;
        int cursor_y = SCREEN_HEIGHT - h - 1; 

        int rect_y = SCREEN_HEIGHT - h - 3;
        int rect_h = h + 4;
        display->fillRect(0, rect_y, SCREEN_WIDTH, rect_h, SSD1306_WHITE);

        display->setTextColor(SSD1306_BLACK);
        display->setCursor(cursor_x, cursor_y);
        display->print(text.c_str());
    };

    draw_single_prompt(0, OLED3_TCA_CHANNEL, props.back_text);
    draw_single_prompt(1, OLED2_TCA_CHANNEL, props.enter_text);
    draw_single_prompt(2, OLED1_TCA_CHANNEL, props.down_text);
}