// src/presentation/blocks/MenuBlock.cpp
// MODIFIED FILE
#include "MenuBlock.h"
#include <Arduino.h>

void MenuBlock::draw(Adafruit_GFX* display, const MenuBlockProps& props) {
    if (!props.is_enabled) {
        return;
    }

    display->setTextSize(1);
    display->setFont(nullptr);
    display->fillRect(0, 19, 128, 45, 0);

    if (props.items.empty()) {
        display->setCursor(4, 30);
        display->setTextColor(1);
        display->print("No menu items.");
        return;
    }

    const int y_top = 22;
    const int y_middle = 34;
    const int y_bottom = 46;

    // --- Draw Selected Item (Always in the middle) ---
    display->fillRect(0, y_middle - 2, 128, 12, 1);
    display->setTextColor(0);
    display->setCursor(4, y_middle);
    display->print(props.items[props.selected_index].c_str());
    display->setTextColor(1);

    // --- Draw Item Above Selection (if it exists) ---
    if (props.selected_index > 0) {
        display->setCursor(4, y_top);
        display->print(props.items[props.selected_index - 1].c_str());
    }

    // --- Draw Item Below Selection (if it exists) ---
    if (props.selected_index < props.items.size() - 1) {
        display->setCursor(4, y_bottom);
        display->print(props.items[props.selected_index + 1].c_str());
    }
}