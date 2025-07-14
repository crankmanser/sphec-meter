// src/presentation/blocks/ProgressBarBlock.h
// NEW FILE
#pragma once

#include <Adafruit_GFX.h>
#include <string>

// This struct holds all the data needed to render a progress bar.
struct ProgressBarBlockProps {
    bool is_enabled = false;
    std::string title;
    int progress_percent = 0;
};

class ProgressBarBlock {
public:
    // The static 'draw' method makes this a stateless utility class.
    static void draw(Adafruit_GFX* display, const ProgressBarBlockProps& props);
};