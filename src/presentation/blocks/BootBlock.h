// src/presentation/blocks/BootBlock.h
// NEW FILE
#pragma once

#include <string>
#include "presentation/DisplayManager.h"

// This struct holds all the data needed to render the boot screen.
struct BootBlockProps {
    std::string title;
    std::string message;
    int animation_step = 0;
};

class BootBlock {
public:
    // The static 'draw' method makes this a stateless utility class.
    // It takes the low-level DisplayManager directly, as it runs before
    // the main UIManager is available.
    static void draw(DisplayManager* displayManager, const BootBlockProps& props);
};