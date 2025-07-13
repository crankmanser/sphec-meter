// src/presentation/blocks/GraphBlock.h
// NEW FILE
#pragma once

#include <Adafruit_GFX.h>
#include <vector>
#include <string>

// This struct holds all the data needed to render a graph.
struct GraphBlockProps {
    bool is_enabled = false;
    std::string title;
    const std::vector<float>* data_points = nullptr;
    float y_min = 0.0f;
    float y_max = 0.0f;
    bool auto_scale_y = true;
};

class GraphBlock {
public:
    static void draw(Adafruit_GFX* display, const GraphBlockProps& props);
};