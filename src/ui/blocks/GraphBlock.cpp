// File Path: /src/ui/blocks/GraphBlock.cpp
// MODIFIED FILE

#include "GraphBlock.h"
#include <algorithm>
#include <Adafruit_SSD1306.h>

/**
 * @brief Draws a time-series graph on the provided display.
 * This function handles both auto-scaling and manual scaling of the Y-axis.
 */
void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    if (!props.is_enabled || !display) return;
    if (!props.pre_filter_data && !props.post_filter_data) return;

    const int graph_x = 0;
    const int graph_y = 12;
    const int graph_width = 128;
    const int graph_height = 34;

    double min_val, max_val;

    // --- DEFINITIVE FIX: Implement flexible Y-axis scaling ---
    // If the override is enabled, use the manually provided min/max values.
    if (props.y_axis_override_enabled) {
        min_val = props.y_min_override;
        max_val = props.y_max_override;
    } else {
        // Otherwise, perform the standard auto-scaling by finding the
        // min and max values within the provided datasets.
        min_val = props.pre_filter_data ? props.pre_filter_data[0] : props.post_filter_data[0];
        max_val = min_val;
        for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
            if (props.pre_filter_data) { min_val = std::min(min_val, props.pre_filter_data[i]); max_val = std::max(max_val, props.pre_filter_data[i]); }
            if (props.post_filter_data) { min_val = std::min(min_val, props.post_filter_data[i]); max_val = std::max(max_val, props.post_filter_data[i]); }
            if (props.ghost_filter_data) { min_val = std::min(min_val, props.ghost_filter_data[i]); max_val = std::max(max_val, props.ghost_filter_data[i]); }
        }
    }

    double y_range = max_val - min_val;
    // Prevent division by zero if the signal is perfectly flat
    if (y_range < 0.1) {
        y_range = 0.1;
        min_val -= 0.05;
    }

    // Lambda function to map a data value to a screen Y-coordinate
    auto map_y = [&](double val) {
        return graph_y + graph_height - static_cast<int>(((val - min_val) / y_range) * (graph_height -1));
    };

    // Draw the graph lines
    for (int i = 0; i < graph_width - 1; ++i) {
        int x0 = graph_x + i;
        int x1 = graph_x + i + 1;
        // Draw pre-filter data as a dotted line
        if (props.pre_filter_data) {
            int y0_pre = map_y(props.pre_filter_data[i]);
            if (i % 4 < 2) display->drawPixel(x0, y0_pre, SSD1306_WHITE);
        }
        // Draw ghost data as a dashed line
        if (props.ghost_filter_data) {
            int y0_ghost = map_y(props.ghost_filter_data[i]);
            int y1_ghost = map_y(props.ghost_filter_data[i+1]);
            if (i % 8 < 4) { display->drawLine(x0, y0_ghost, x1, y1_ghost, SSD1306_WHITE); }
        }
        // Draw post-filter data as a solid line
        if (props.post_filter_data) {
            int y0_post = map_y(props.post_filter_data[i]);
            int y1_post = map_y(props.post_filter_data[i+1]);
            display->drawLine(x0, y0_post, x1, y1_post, SSD1306_WHITE);
        }
    }

    // Draw text labels
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    if (!props.top_left_label.empty()) {
        display->setCursor(2, 2);
        display->print(props.top_left_label.c_str());
    }
    if (!props.bottom_left_label.empty()) {
        display->setCursor(2, 44);
        display->print(props.bottom_left_label.c_str());
    }
    if (!props.bottom_right_label.empty()) {
        display->setCursor(70, 44);
        display->print(props.bottom_right_label.c_str());
    }
}