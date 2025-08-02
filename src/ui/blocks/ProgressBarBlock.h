// File Path: /src/ui/blocks/ProgressBarBlock.h
// NEW FILE

#ifndef PROGRESS_BAR_BLOCK_H
#define PROGRESS_BAR_BLOCK_H

#include <Adafruit_GFX.h>
#include <string>

/**
 * @struct ProgressBarProps
 * @brief Holds the properties needed to render a progress bar.
 */
struct ProgressBarProps {
    bool is_enabled = false;
    int progress_percent = 0; // A value from 0 to 100
    std::string label;        // Optional text label to display above the bar
};

/**
 * @class ProgressBarBlock
 * @brief A stateless, reusable UI component for drawing a progress bar.
 */
class ProgressBarBlock {
public:
    static void draw(Adafruit_GFX* display, const ProgressBarProps& props);
};

#endif // PROGRESS_BAR_BLOCK_H