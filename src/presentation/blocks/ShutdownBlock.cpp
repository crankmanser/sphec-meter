// src/presentation/blocks/ShutdownBlock.cpp
// NEW FILE
#include "ShutdownBlock.h"

void ShutdownBlock::draw(DisplayManager* displayManager, const ShutdownBlockProps& props) {
    if (!displayManager) {
        return;
    }

    // --- Prepare Displays ---
    // Clear all screens to provide a clean, focused shutdown message.
    displayManager->clearAll();

    // The main content will be drawn on the middle OLED.
    Adafruit_SSD1306* display = displayManager->getDisplay(OLED_ID::OLED_MIDDLE);
    displayManager->selectOLED(OLED_ID::OLED_MIDDLE);

    // --- Draw Message ---
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    int16_t x1, y1;
    uint16_t w, h;
    display->getTextBounds(props.message.c_str(), 0, 0, &x1, &y1, &w, &h);
    display->setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
    display->print(props.message.c_str());

    // --- Show the Frame ---
    // Push the buffer to all screens to ensure they are cleared and the
    // middle screen shows the message.
    displayManager->displayAll();
}