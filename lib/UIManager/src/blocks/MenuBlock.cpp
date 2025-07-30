// File Path: /lib/UIManager/src/blocks/MenuBlock.cpp
// NEW FILE

#include "MenuBlock.h"

/**
 * @brief Draws a menu component onto the provided display buffer.
 *
 * This function renders a three-item-view menu, with the selected item
 * highlighted in the center. This visual style is migrated from the
 * well-received legacy v1.6.6 UI.
 * @param display Pointer to the Adafruit_GFX object for the target OLED.
 * @param props The properties describing the menu to be drawn.
 */
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

    // Define the vertical positions for the three visible menu items.
    const int y_top = 22;
    const int y_middle = 34;
    const int y_bottom = 46;

    // --- Draw Selected Item (Always in the middle, highlighted) ---
    display->fillRect(0, y_middle - 2, 128, 12, 1); // White highlight box
    display->setTextColor(0); // Black text on white background
    display->setCursor(4, y_middle);
    display->print(props.items[props.selected_index].c_str());
    display->setTextColor(1); // Reset to white text for other items

    // --- Draw Item Above Selection (if one exists) ---
    if (props.selected_index > 0) {
        display->setCursor(4, y_top);
        display->print(props.items[props.selected_index - 1].c_str());
    }

    // --- Draw Item Below Selection (if one exists) ---
    if (props.selected_index < props.items.size() - 1) {
        display->setCursor(4, y_bottom);
        display->print(props.items[props.selected_index + 1].c_str());
    }
}