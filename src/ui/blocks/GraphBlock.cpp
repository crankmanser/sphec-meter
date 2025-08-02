// File Path: /src/ui/blocks/GraphBlock.cpp
// MODIFIED FILE

#include "GraphBlock.h"
#include <algorithm>

/**
 * @brief Draws a graph on the provided display.
 * @param display A pointer to the Adafruit_GFX display object.
 * @param props The properties defining the graph's data and labels.
 */
void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    // --- FIX: Modified guard to allow drawing even if one dataset is null ---
    if (!props.is_enabled || !display || (!props.pre_filter_data && !props.post_filter_data)) {
        return;
    }

    const int graph_x = 0;
    const int graph_y = 10;
    const int graph_width = 128;
    const int graph_height = 44;

    // Step 1: Find the min and max values to auto-scale the Y-axis.
    // Initialize with a value from whichever buffer is not null.
    double min_val = props.pre_filter_data ? props.pre_filter_data[0] : props.post_filter_data[0];
    double max_val = min_val;

    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
        if (props.pre_filter_data) {
            min_val = std::min(min_val, props.pre_filter_data[i]);
            max_val = std::max(max_val, props.pre_filter_data[i]);
        }
        if (props.post_filter_data) {
            min_val = std::min(min_val, props.post_filter_data[i]);
            max_val = std::max(max_val, props.post_filter_data[i]);
        }
    }

    double y_range = max_val - min_val;
    if (y_range < 0.1) {
        y_range = 0.1;
        min_val -= 0.05;
    }

    auto map_y = [&](double val) {
        return graph_y + graph_height - static_cast<int>(((val - min_val) / y_range) * (graph_height -1));
    };

    // Step 2: Draw the two data series
    for (int i = 0; i < graph_width - 1; ++i) {
        int x0 = graph_x + i;
        int x1 = graph_x + i + 1;

        // --- FIX: Check for null before drawing pre-filter data (dotted line) ---
        if (props.pre_filter_data) {
            int y0_pre = map_y(props.pre_filter_data[i]);
            if (i % 4 < 2) {
                 display->drawPixel(x0, y0_pre, 1);
            }
        }
        
        // --- FIX: Check for null before drawing post-filter data (solid line) ---
        if (props.post_filter_data) {
            int y0_post = map_y(props.post_filter_data[i]);
            int y1_post = map_y(props.post_filter_data[i+1]);
            display->drawLine(x0, y0_post, x1, y1_post, 1);
        }
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