// src/presentation/blocks/GraphBlock.cpp
// MODIFIED FILE
#include "GraphBlock.h"
#include <algorithm>

void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    if (!props.is_enabled || !props.data_points || props.data_points->size() < 2) {
        return;
    }

    // <<< MODIFIED: Graph area is now dynamic >>>
    const int graph_x = 0;
    const int graph_y = props.show_top_bar ? 19 : 0;
    const int graph_w = 128;
    // Use full height minus the prompt area at the bottom
    const int graph_h = 54 - graph_y; 

    // Draw Graph Area Outline
    display->drawRect(graph_x, graph_y, graph_w, graph_h, 1);

    float min_val = props.y_min;
    float max_val = props.y_max;

    if (props.auto_scale_y) {
        min_val = *std::min_element(props.data_points->begin(), props.data_points->end());
        max_val = *std::max_element(props.data_points->begin(), props.data_points->end());
    }

    float y_range = max_val - min_val;
    if (y_range < 1e-6) y_range = 1.0; 

    // Plot Data
    int last_x = 0;
    int last_y = 0;
    for (size_t i = 0; i < props.data_points->size(); ++i) {
        // <<< FIX: Multiply by 1000 to preserve floating point precision for map() >>>
        long val_map = (*props.data_points)[i] * 1000;
        long min_map = min_val * 1000;
        long max_map = max_val * 1000;

        int current_x = map(i, 0, props.data_points->size() - 1, graph_x, graph_x + graph_w - 1);
        int current_y = graph_y + graph_h - map(val_map, min_map, max_map, 0, graph_h -1);

        if (i > 0) {
            display->drawLine(last_x, last_y, current_x, current_y, 1);
        }
        last_x = current_x;
        last_y = current_y;
    }
}