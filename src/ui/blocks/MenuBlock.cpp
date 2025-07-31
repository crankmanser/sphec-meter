// File Path: /src/ui/blocks/MenuBlock.cpp
// MODIFIED FILE

#include "MenuBlock.h"

void MenuBlock::draw(Adafruit_GFX* display, const MenuBlockProps& props) {
    if (!props.is_enabled || !display) {
        return;
    }

    display->setTextSize(1);
    display->setFont(nullptr);

    if (props.items.empty()) {
        display->setCursor(4, 30);
        display->setTextColor(1);
        display->print("No menu items.");
        return;
    }

    const int y_top = 22;
    const int y_middle = 34;
    const int y_bottom = 46;

    // Draw Selected Item (Always in the middle, highlighted)
    display->fillRect(0, y_middle - 2, 128, 12, 1);
    display->setTextColor(0);
    display->setCursor(4, y_middle);
    display->print(props.items[props.selected_index].c_str());
    display->setTextColor(1);

    // Draw Item Above Selection
    if (props.selected_index > 0) {
        display->setCursor(4, y_top);
        display->print(props.items[props.selected_index - 1].c_str());
    }

    // Draw Item Below Selection
    // --- FIX: Corrected typo from "insex" to "index" ---
    if (props.selected_index < props.items.size() - 1) {
        display->setCursor(4, y_bottom);
        display->print(props.items[props.selected_index + 1].c_str());
    }
}
