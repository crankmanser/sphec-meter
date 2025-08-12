// File Path: /src/ui/blocks/CalibrationCurveBlock.h
// NEW FILE

#ifndef CALIBRATION_CURVE_BLOCK_H
#define CALIBRATION_CURVE_BLOCK_H

#include <Adafruit_GFX.h>
#include "CalibrationManager.h" // For CalibrationModel struct

/**
 * @struct CalibrationCurveBlockProps
 * @brief Holds the properties needed to render the calibration curve graph.
 */
struct CalibrationCurveBlockProps {
    bool is_enabled = false;
    const CalibrationModel* model = nullptr;
    double live_voltage = 0.0;
    double live_value = 0.0;
};

/**
 * @class CalibrationCurveBlock
 * @brief A specialized, stateless UI block for rendering the quadratic
 * calibration curve and a live reading crosshair.
 */
class CalibrationCurveBlock {
public:
    static void draw(Adafruit_GFX* display, const CalibrationCurveBlockProps& props);
};

#endif // CALIBRATION_CURVE_BLOCK_H