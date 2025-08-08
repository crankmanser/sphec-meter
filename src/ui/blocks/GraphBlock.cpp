// File Path: /src/ui/blocks/GraphBlock.cpp
// MODIFIED FILE

#include "GraphBlock.h"
#include <algorithm>
#include <Adafruit_SSD1306.h>

void GraphBlock::draw(Adafruit_GFX* display, const GraphBlockProps& props) {
    if (!props.is_enabled || !display) return;
    if (!props.pre_filter_data && !props.post_filter_data) return;

    const int graph_x = 0;
    const int graph_y = 12; 
    const int graph_width = 128;
    const int graph_height = 34;

    double min_val = props.pre_filter_data ? props.pre_filter_data[0] : props.post_filter_data[0];
    double max_val = min_val;
    for (int i = 0; i < GRAPH_DATA_POINTS; ++i) {
        if (props.pre_filter_data) { min_val = std::min(min_val, props.pre_filter_data[i]); max_val = std::max(max_val, props.pre_filter_data[i]); }
        if (props.post_filter_data) { min_val = std::min(min_val, props.post_filter_data[i]); max_val = std::max(max_val, props.post_filter_data[i]); }
        if (props.ghost_filter_data) { min_val = std::min(min_val, props.ghost_filter_data[i]); max_val = std::max(max_val, props.ghost_filter_data[i]); }
    }
    double y_range = max_val - min_val;
    if (y_range < 0.1) { y_range = 0.1; min_val -= 0.05; }
    auto map_y = [&](double val) { return graph_y + graph_height - static_cast<int>(((val - min_val) / y_range) * (graph_height -1)); };

    for (int i = 0; i < graph_width - 1; ++i) {
        int x0 = graph_x + i;
        int x1 = graph_x + i + 1;
        if (props.pre_filter_data) { int y0_pre = map_y(props.pre_filter_data[i]); if (i % 4 < 2) display->drawPixel(x0, y0_pre, SSD1306_WHITE); }
        if (props.ghost_filter_data) {
            int y0_ghost = map_y(props.ghost_filter_data[i]);
            int y1_ghost = map_y(props.ghost_filter_data[i+1]);
            if (i % 8 < 4) { display->drawLine(x0, y0_ghost, x1, y1_ghost, SSD1306_WHITE); }
        }
        if (props.post_filter_data) { int y0_post = map_y(props.post_filter_data[i]); int y1_post = map_y(props.post_filter_data[i+1]); display->drawLine(x0, y0_post, x1, y1_post, SSD1306_WHITE); }
    }

    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    if (!props.top_left_label.empty()) {
        display->setCursor(2, 2);
        display->print(props.top_left_label.c_str());
    }
    // --- DEFINITIVE FIX: Adjust Y-coordinate to prevent overlap with button prompts ---
    if (!props.bottom_left_label.empty()) {
        display->setCursor(2, 48);
        display->print(props.bottom_left_label.c_str());
    }
    if (!props.bottom_right_label.empty()) {
        display->setCursor(70, 48);
        display->print(props.bottom_right_label.c_str());
    }
}