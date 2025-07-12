#pragma once

#include "helpers/calibration/CalibrationModel.h"
#include "config/calibration_config.h" // For PH_CAL_POINT_LOW, etc.
#include "helpers/calibration/CalibrationEngine.h" // Needed for CalibrationEngine::calculateModel in initialization

namespace pHCalibrationDefaults {

    // --- Predefined pH Calibration Model ---
    // This model provides a default, approximate pH calibration.
    // It will be used if no saved calibration is found on the SD card.
    // The device will also set an 'is_valid = false' flag internally to indicate it's using a default.

    // Using approximate voltage points for common pH buffers (pH 4.01, 6.86, 9.18)
    // These voltage values are typical for a pH probe and amplifier setup.
    // pH 4.01 typically around 3.0V (acidic side)
    // pH 6.86 typically around 2.5V (neutral, theoretical 2.5V center for 0-5V swing)
    // pH 9.18 typically around 2.0V (alkaline side)
    const CalibrationModel DEFAULT_MODEL = []{
        CalibrationModel model;
        
        float v_ph4_default = 3.0f;
        float v_ph7_default = 2.5f;
        float v_ph9_default = 2.0f;
        
        model = CalibrationEngine::calculateModel(
            v_ph4_default, PH_CAL_POINT_LOW,
            v_ph7_default, PH_CAL_POINT_MID,
            v_ph9_default, PH_CAL_POINT_HIGH
        );
        
        // Mark as valid only if calculation was successful (should be for these points)
        // If calculation fails, is_valid will remain false from CalibrationEngine.
        if (model.is_valid) {
            model.quality_score = 100.0f; // Assume perfect quality for this predefined default
            model.last_sensor_drift = 0.0f; // No drift for a default model
        }
        return model;
    }();

} // namespace pHCalibrationDefaults