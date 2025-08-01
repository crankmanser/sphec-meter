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

    // --- FIX: Y positions moved down to create space for the status area above ---
    const int y_top = 28;
    const int y_middle = 40;
    const int y_bottom = 52;
    const int highlight_height = 12;

    // Draw Selected Item
    display->fillRect(0, y_middle - (highlight_height / 2) - 2, 128, highlight_height, 1);
    display->setTextColor(0);
    display->setCursor(4, y_middle - 4);
    display->print(props.items[props.selected_index].c_str());
    display->setTextColor(1);

    // Draw Item Above Selection
    if (props.selected_index > 0) {
        display->setCursor(4, y_top - 4);
        display->print(props.items[props.selected_index - 1].c_str());
    }

    // Draw Item Below Selection
    if (props.selected_index < props.items.size() - 1) {
        display->setCursor(4, y_bottom - 4);
        display->print(props.items[props.selected_index + 1].c_str());
    }
}