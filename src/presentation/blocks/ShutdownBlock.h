// src/presentation/blocks/ShutdownBlock.h
// NEW FILE
#pragma once

#include <string>
#include "presentation/DisplayManager.h"

// This struct holds the data needed to render the shutdown screen.
struct ShutdownBlockProps {
    std::string message;
};

/**
 * @class ShutdownBlock
 * @brief A stateless UI block for displaying a shutdown screen.
 *
 * This block is called directly with the DisplayManager, as the main
 * UI loop and UIManager may not be running during a shutdown sequence.
 */
class ShutdownBlock {
public:
    static void draw(DisplayManager* displayManager, const ShutdownBlockProps& props);
};