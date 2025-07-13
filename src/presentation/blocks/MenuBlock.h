// src/presentation/blocks/MenuBlock.h
// NEW FILE
#pragma once

#include <Adafruit_GFX.h>
#include <vector>
#include <string>

// This struct holds all the data needed to render a menu.
// A screen will create and populate this struct.
struct MenuBlockProps {
    bool is_enabled = false; // Is this block active for a given OLED?
    std::vector<std::string> items;
    int selected_index = 0;
};

class MenuBlock {
public:
    // The static 'draw' method makes this a stateless utility class.
    // It takes the display driver and the properties to render.
    static void draw(Adafruit_GFX* display, const MenuBlockProps& props);
};