// File Path: /src/ui/blocks/MenuBlock.h
// NEW FILE

#ifndef MENU_BLOCK_H
#define MENU_BLOCK_H

#include <Adafruit_GFX.h>
#include <vector>
#include <string>

/**
 * @struct MenuBlockProps
 * @brief A data structure holding all the properties needed to render a menu.
 * A screen will create and populate this struct to describe the menu it wants.
 */
struct MenuBlockProps {
    bool is_enabled = false;
    std::vector<std::string> items;
    int selected_index = 0;
};

/**
 * @class MenuBlock
 * @brief A stateless, reusable UI component for drawing menus.
 *
 * This is a classic example of the "Block-Based Assembly" pattern. It has a single
 * static `draw` method that takes the display driver and the properties to render.
 */
class MenuBlock {
public:
    static void draw(Adafruit_GFX* display, const MenuBlockProps& props);
};

#endif // MENU_BLOCK_H