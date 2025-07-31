// File Path: /src/ui/blocks/GraphBlock.h
// NEW FILE

#ifndef GRAPH_BLOCK_H
#define GRAPH_BLOCK_H

#include <Adafruit_GFX.h>
#include <string>

// The number of data points the graph will display.
#define GRAPH_DATA_POINTS 128

/**
 * @struct GraphBlockProps
 * @brief A data structure holding all properties to render a dual-line graph.
 */
struct GraphBlockProps {
    bool is_enabled = false;

    // Pointers to the data buffers to be displayed.
    // The graph block does not own this data, it only reads from it.
    const double* pre_filter_data = nullptr;
    const double* post_filter_data = nullptr;

    // Text labels to be overlaid on the graph area.
    std::string top_left_label;
    std::string top_right_label;
    std::string bottom_left_label;
    std::string bottom_right_label;
};

/**
 * @class GraphBlock
 * @brief A stateless, reusable UI component for drawing line graphs.
 *
 * This block can draw two data series on the same graph: a dotted "pre-filter"
 * line and a solid "post-filter" line. It automatically scales the data
 * to fit the available display area.
 */
class GraphBlock {
public:
    static void draw(Adafruit_GFX* display, const GraphBlockProps& props);
};

#endif // GRAPH_BLOCK_H