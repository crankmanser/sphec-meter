// src/presentation/blocks/BootBlock.cpp
// NEW FILE
#include "BootBlock.h"

void BootBlock::draw(DisplayManager* displayManager, const BootBlockProps& props) {
    if (!displayManager) {
        return;
    }

    // --- Prepare Displays ---
    // We clear all three displays to ensure a clean boot screen.
    displayManager->clearAll();

    // The main content will be drawn on the middle OLED.
    Adafruit_SSD1306* display = displayManager->getDisplay(OLED_ID::OLED_MIDDLE);
    displayManager->selectOLED(OLED_ID::OLED_MIDDLE);

    // --- Draw Title ---
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(props.title.c_str(), 0, 0, &x1, &y1, &w, &h);
    display->setCursor((SCREEN_WIDTH - w) / 2, 10);
    display->print(props.title.c_str());

    // --- Draw Message & Animation ---
    std::string message_with_anim = props.message;
    for (int i = 0; i < props.animation_step; ++i) {
        message_with_anim += ".";
    }

    display->setTextSize(1);
    display->getTextBounds(message_with_anim.c_str(), 0, 0, &x1, &y1, &w, &h);
    display->setCursor((SCREEN_WIDTH - w) / 2, 40);
    display->print(message_with_anim.c_str());


    // --- Show the Frame ---
    // We only need to call display() on the middle screen since the others are blank.
    displayManager->displayOLED(OLED_ID::OLED_MIDDLE);
}