#pragma once

#include "helpers/calibration/CalibrationModel.h" // For CalibrationModel struct
#include "helpers/calibration/CalibrationEngine.h" // For SensorType enum in CalibrationModel
#include "config/calibration_config.h"             // For PH_CAL_POINT_MID etc.
#include <Arduino.h>                               // For basic types like float, int16_t

/**
 * @class PhCalibrationSimulator
 * @brief Provides simulated pH calibration models and raw ADC values for debugging.
 *
 * This class is part of the calibration simulation feature. It allows the
 * pH sensor pipeline to be tested end-to-end without physical hardware.
 * All methods are static as this class primarily serves as a data provider.
 */
class PhCalibrationSimulator {
public:
    /**
     * @brief Provides a simulated, valid pH calibration model.
     * This model is designed to produce sensible pH values around neutral (7.0).
     * @return A CalibrationModel struct representing a valid pH calibration.
     */
    static CalibrationModel getSimulatedCalibrationModel();

    /**
     * @brief Provides a simulated raw ADC value for a neutral pH.
     * This value corresponds to approximately 2.5V at the ADC input for pH 7.0.
     * @return A simulated raw 16-bit integer ADC value.
     */
    static int16_t getSimulatedRawADC(); 

    // Future: Method to get simulated raw ADC value for a specific target pH value.
    // static int16_t getSimulatedRawADC(float target_ph_value);
};