// File Path: /src/ui/blocks/GraphBlock.h
// MODIFIED FILE

#ifndef GRAPH_BLOCK_H
#define GRAPH_BLOCK_H

#include <Adafruit_GFX.h>
#include <string>
#include "PI_Filter.h" // For FILTER_HISTORY_SIZE

#define GRAPH_DATA_POINTS FILTER_HISTORY_SIZE

/**
 * @struct GraphBlockProps
 * @brief A data structure holding all the properties needed to render a time-series graph.
 */
struct GraphBlockProps {
    // Core properties
    bool is_enabled = false;
    const double* pre_filter_data = nullptr;  // The "noisy" raw signal
    const double* post_filter_data = nullptr; // The final, filtered signal
    const double* ghost_filter_data = nullptr; // An optional, static "ghost" line for comparison

    // Text label properties
    std::string top_left_label;
    std::string top_right_label;
    std::string bottom_left_label;
    std::string bottom_right_label;

    // --- NEW: Manual Y-Axis Scaling ---
    // These properties allow for overriding the default auto-scaling behavior
    // to provide a "zoomed-out" view of the graph.
    bool y_axis_override_enabled = false;
    double y_min_override = 0.0;
    double y_max_override = 0.0;
};

/**
 * @class GraphBlock
 * @brief A stateless, reusable UI component for drawing time-series graphs.
 */
class GraphBlock {
public:
    static void draw(Adafruit_GFX* display, const GraphBlockProps& props);
};

#endif // GRAPH_BLOCK_H