// src/presentation/blocks/ProgressBarBlock.cpp
// NEW FILE
#include "ProgressBarBlock.h"

void ProgressBarBlock::draw(Adafruit_GFX* display, const ProgressBarBlockProps& props) {
    if (!props.is_enabled) {
        return;
    }

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(1);

    // --- Draw Title ---
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(props.title.c_str(), 0, 0, &x1, &y1, &w, &h);
    int text_x = (display->width() - w) / 2;
    display->setCursor(text_x, 24);
    display->print(props.title.c_str());

    // --- Draw Progress Bar ---
    int bar_x = 14;
    int bar_y = 38;
    int bar_w = 100;
    int bar_h = 10;
    display->drawRect(bar_x, bar_y, bar_w, bar_h, 1);

    int progress_w = (props.progress_percent * (bar_w - 4)) / 100;
    display->fillRect(bar_x + 2, bar_y + 2, progress_w, bar_h - 4, 1);
}