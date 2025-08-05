// File Path: /src/ui/blocks/MenuBlock.cpp
// MODIFIED FILE

#include "MenuBlock.h"

void MenuBlock::draw(Adafruit_GFX* display, const MenuBlockProps& props) {
    if (!props.is_enabled || !display || props.items.empty()) {
        return;
    }

    display->setTextSize(1);
    display->setFont(nullptr);

    const int y_top = 32;
    const int y_middle = 44;
    const int y_bottom = 56;
    const int highlight_height = 12;

    // --- DEFINITIVE FIX: Restore full-width highlight for menu selection ---
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