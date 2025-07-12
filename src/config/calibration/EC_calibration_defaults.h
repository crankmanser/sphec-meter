#pragma once

#include "helpers/calibration/CalibrationModel.h"
#include "config/calibration_config.h" // For EC_CAL_POINT_LOW, etc.
#include "helpers/calibration/CalibrationEngine.h" // Needed for CalibrationEngine::calculateModel in initialization

namespace ECCalibrationDefaults {

    // --- Predefined EC Calibration Model ---
    // This model provides a default, approximate EC calibration.
    // It will be used if no saved calibration is found on the SD card.
    // The device will also set an 'is_valid = false' flag internally to indicate it's using a default.

    // Using approximate voltage points for common EC buffers (84 uS/cm, 1413 uS/cm, 12880 uS/cm)
    // These voltage values are typical for an EC probe and amplifier setup.
    // EC 84 uS/cm typically around 0.1V (low conductivity)
    // EC 1413 uS/cm typically around 1.0V (mid conductivity)
    // EC 12880 uS/cm typically around 2.5V (high conductivity)
    const CalibrationModel DEFAULT_MODEL = []{
        CalibrationModel model;
        
        float v_ec84_default = 0.1f;
        float v_ec1413_default = 1.0f;
        float v_ec12880_default = 2.5f;
        
        model = CalibrationEngine::calculateModel(
            v_ec84_default, EC_CAL_POINT_LOW,
            v_ec1413_default, EC_CAL_POINT_MID,
            v_ec12880_default, EC_CAL_POINT_HIGH
        );
        
        // Mark as valid only if calculation was successful (should be for these points)
        // If calculation fails, is_valid will remain false from CalibrationEngine.
        if (model.is_valid) {
            model.quality_score = 100.0f; // Assume perfect quality for this predefined default
            model.last_sensor_drift = 0.0f; // No drift for a default model
        }
        return model;
    }();

} // namespace ECCalibrationDefaults