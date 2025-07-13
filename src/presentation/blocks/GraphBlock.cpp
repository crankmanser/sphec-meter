// src/presentation/blocks/GraphBlock.cpp
// NEW FILE
#include "GraphBlock.h"
#include <algorithm> // For std::min_element, std::max_element

void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    if (!props.is_enabled || !props.data_points || props.data_points->size() < 2) {
        return;
    }

    const int graph_x = 0, graph_y = 20, graph_w = 128, graph_h = 32;

    // Draw Title
    display->setTextSize(1);
    display->setTextColor(1);
    display->setCursor(graph_x, graph_y - 10);
    display->print(props.title.c_str());

    // Draw Graph Area Outline
    display->drawRect(graph_x, graph_y, graph_w, graph_h, 1);

    float min_val = props.y_min;
    float max_val = props.y_max;

    if (props.auto_scale_y) {
        min_val = *std::min_element(props.data_points->begin(), props.data_points->end());
        max_val = *std::max_element(props.data_points->begin(), props.data_points->end());
    }

    float y_range = max_val - min_val;
    if (y_range == 0) y_range = 1.0; // Avoid division by zero

    // Plot Data
    for (size_t i = 0; i < props.data_points->size() - 1; ++i) {
        float data_curr = (*props.data_points)[i];
        float data_next = (*props.data_points)[i+1];

        int x1 = map(i, 0, props.data_points->size() - 1, graph_x, graph_x + graph_w);
        int y1 = graph_y + graph_h - map(data_curr, min_val, max_val, 0, graph_h);

        int x2 = map(i + 1, 0, props.data_points->size() - 1, graph_x, graph_x + graph_w);
        int y2 = graph_y + graph_h - map(data_next, min_val, max_val, 0, graph_h);
        
        display->drawLine(x1, y1, x2, y2, 1);
    }
}