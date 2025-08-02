// File Path: /src/ui/blocks/ProgressBarBlock.cpp
// NEW FILE

#include "ProgressBarBlock.h"

/**
 * @brief Draws a progress bar on the provided display.
 * @param display A pointer to the Adafruit_GFX display object.
 * @param props The properties defining the progress bar's state.
 */
void ProgressBarBlock::draw(Adafruit_GFX* display, const ProgressBarProps& props) {
    if (!props.is_enabled || !display) {
        return;
    }

    const int bar_x = 4;
    const int bar_y = 40;
    const int bar_width = 120;
    const int bar_height = 12;

    // Draw the label above the bar if it exists
    if (!props.label.empty()) {
        display->setTextSize(1);
        display->setFont(nullptr);
        display->setTextColor(1);
        display->setCursor(4, 28);
        display->print(props.label.c_str());
    }

    // Draw the outer border of the progress bar
    display->drawRect(bar_x, bar_y, bar_width, bar_height, 1);

    // Calculate the width of the filled portion of the bar
    int progress_width = map(props.progress_percent, 0, 100, 0, bar_width - 2);
    if (progress_width > 0) {
        display->fillRect(bar_x + 1, bar_y + 1, progress_width, bar_height - 2, 1);
    }
}