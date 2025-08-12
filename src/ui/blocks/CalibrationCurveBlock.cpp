// File Path: /src/ui/blocks/CalibrationCurveBlock.cpp
// MODIFIED FILE

#include "CalibrationCurveBlock.h"
#include <algorithm> // For std::min/max
// --- DEFINITIVE FIX: Include the required header for color definitions ---
#include <Adafruit_SSD1306.h>

void CalibrationCurveBlock::draw(Adafruit_GFX* display, const CalibrationCurveBlockProps& props) {
    if (!props.is_enabled || !display || !props.model || !props.model->isCalibrated) {
        display->setTextSize(1);
        display->setCursor(10, 30);
        display->print("Not Calibrated");
        return;
    }

    const int graph_x = 0;
    const int graph_y = 0;
    const int graph_width = 128;
    const int graph_height = 64;

    // Determine graph bounds from the model's calibration points
    double min_v = props.model->points[0].voltage;
    double max_v = props.model->points[0].voltage;
    double min_val = props.model->points[0].value;
    double max_val = props.model->points[0].value;

    for (int i = 1; i < CALIBRATION_POINT_COUNT; ++i) {
        min_v = std::min(min_v, props.model->points[i].voltage);
        max_v = std::max(max_v, props.model->points[i].voltage);
        min_val = std::min(min_val, props.model->points[i].value);
        max_val = std::max(max_val, props.model->points[i].value);
    }
    // Add some padding to the bounds
    min_v -= (max_v - min_v) * 0.1;
    max_v += (max_v - min_v) * 0.1;
    min_val -= (max_val - min_val) * 0.1;
    max_val += (max_val - min_val) * 0.1;


    double v_range = max_v - min_v;
    double val_range = max_val - min_val;
    if (v_range < 1e-6) v_range = 1.0;
    if (val_range < 1e-6) val_range = 1.0;

    // Lambda functions to map model coordinates to screen coordinates
    auto map_x = [&](double voltage) {
        return graph_x + static_cast<int>(((voltage - min_v) / v_range) * graph_width);
    };
    auto map_y = [&](double value) {
        return graph_y + graph_height - static_cast<int>(((value - min_val) / val_range) * graph_height);
    };

    // 1. Draw the quadratic curve
    for (int sx = 0; sx < graph_width; ++sx) {
        double v1 = min_v + (sx * v_range / graph_width);
        double v2 = min_v + ((sx + 1) * v_range / graph_width);
        double val1 = (props.model->coeff_a * v1 * v1) + (props.model->coeff_b * v1) + props.model->coeff_c;
        double val2 = (props.model->coeff_a * v2 * v2) + (props.model->coeff_b * v2) + props.model->coeff_c;
        display->drawLine(map_x(v1), map_y(val1), map_x(v2), map_y(val2), SSD1306_WHITE);
    }

    // 2. Draw the original calibration points
    for (int i = 0; i < CALIBRATION_POINT_COUNT; ++i) {
        int px = map_x(props.model->points[i].voltage);
        int py = map_y(props.model->points[i].value);
        display->fillRect(px - 1, py - 1, 3, 3, SSD1306_WHITE);
    }

    // 3. Draw the live crosshair
    int cross_x = map_x(props.live_voltage);
    int cross_y = map_y(props.live_value);
    display->drawFastHLine(cross_x - 3, cross_y, 7, SSD1306_INVERSE);
    display->drawFastVLine(cross_x, cross_y - 3, 7, SSD1306_INVERSE);
}