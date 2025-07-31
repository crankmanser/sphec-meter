// File Path: /src/ui/blocks/GraphBlock.cpp
// NEW FILE

#include "GraphBlock.h"
#include <algorithm> // For std::min/max

void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    if (!props.is_enabled || !display || !props.pre_filter_data || !props.post_filter_data) {
        return;
    }

    const int graph_x = 0;
    const int graph_y = 10;
    const int graph_width = 128;
    const int graph_height = 44;

    // Step 1: Find the min and max values in the combined data to auto-scale the Y-axis.
    double min_val = props.pre_filter_data[0];
    double max_val = props.pre_filter_data[0];

    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
        min_val = std::min(min_val, props.pre_filter_data[i]);
        min_val = std::min(min_val, props.post_filter_data[i]);
        max_val = std::max(max_val, props.pre_filter_data[i]);
        max_val = std::max(max_val, props.post_filter_data[i]);
    }

    double y_range = max_val - min_val;
    if (y_range < 0.1) { // Prevent division by zero and ensure a minimum visible range
        y_range = 0.1;
        min_val -= 0.05;
    }

    // Helper lambda to map a data value to a Y-coordinate on the screen
    auto map_y = [&](double val) {
        return graph_y + graph_height - static_cast<int>(((val - min_val) / y_range) * (graph_height -1));
    };

    // Step 2: Draw the two data series
    for (int i = 0; i < graph_width - 1; ++i) {
        int x0 = graph_x + i;
        int x1 = graph_x + i + 1;

        // Draw pre-filter data as a dotted line
        int y0_pre = map_y(props.pre_filter_data[i]);
        if (i % 4 < 2) { // Draw pixel only on even-numbered pairs for dotted effect
             display->drawPixel(x0, y0_pre, 1);
        }

        // Draw post-filter data as a solid line
        int y0_post = map_y(props.post_filter_data[i]);
        int y1_post = map_y(props.post_filter_data[i+1]);
        display->drawLine(x0, y0_post, x1, y1_post, 1);
    }

    // Step 3: Draw the overlay text (KPIs)
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(1);

    if (!props.top_left_label.empty()) {
        display->setCursor(2, 2);
        display->print(props.top_left_label.c_str());
    }
    if (!props.top_right_label.empty()) {
        display->setCursor(70, 2);
        display->print(props.top_right_label.c_str());
    }
    if (!props.bottom_left_label.empty()) {
        display->setCursor(2, 56);
        display->print(props.bottom_left_label.c_str());
    }
    if (!props.bottom_right_label.empty()) {
        display->setCursor(70, 56);
        display->print(props.bottom_right_label.c_str());
    }
}