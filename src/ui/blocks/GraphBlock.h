// File Path: /src/ui/blocks/GraphBlock.h
// MODIFIED FILE

#ifndef GRAPH_BLOCK_H
#define GRAPH_BLOCK_H

#include <Adafruit_GFX.h>
#include <string>
// --- NEW: Include the header that defines the shared constant ---
#include "PI_Filter.h"

// --- FIX: Use the shared constant for the number of data points ---
// This ensures the graph is always sized to match the filter's history buffer.
#define GRAPH_DATA_POINTS FILTER_HISTORY_SIZE

struct GraphBlockProps {
    bool is_enabled = false;
    const double* pre_filter_data = nullptr;
    const double* post_filter_data = nullptr;
    std::string top_left_label;
    std::string top_right_label;
    std::string bottom_left_label;
    std::string bottom_right_label;
};

class GraphBlock {
public:
    static void draw(Adafruit_GFX* display, const GraphBlockProps& props);
};

#endif // GRAPH_BLOCK_H