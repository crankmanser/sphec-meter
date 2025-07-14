// src/presentation/blocks/GraphBlock.h
// MODIFIED FILE
#pragma once

#include <Adafruit_GFX.h>
#include <vector>
#include <string>

struct GraphBlockProps {
    bool is_enabled = false;
    const std::vector<float>* data_points = nullptr;
    float y_min = 0.0f;
    float y_max = 0.0f;
    bool auto_scale_y = true;
    // <<< ADDED: To make the block aware of the status bar >>>
    bool show_top_bar = true; 
};

class GraphBlock {
public:
    static void draw(Adafruit_GFX* display, const GraphBlockProps& props);
};