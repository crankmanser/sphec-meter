#pragma once

#include "helpers/calibration/CalibrationModel.h" // For CalibrationModel struct
#include "helpers/calibration/CalibrationEngine.h" // For SensorType enum in CalibrationModel
#include "config/calibration_config.h"             // For EC_CAL_POINT_MID etc.
#include <Arduino.h>                               // For basic types like float, int16_t

/**
 * @class EcCalibrationSimulator
 * @brief Provides simulated EC calibration models and raw ADC values for debugging.
 *
 * This class is part of the calibration simulation feature. It allows the
 * EC sensor pipeline to be tested end-to-end without physical hardware.
 * All methods are static as this class primarily serves as a data provider.
 */
class EcCalibrationSimulator {
public:
    /**
     * @brief Provides a simulated, valid EC calibration model.
     * This model is designed to produce sensible EC values around neutral (1413 uS/cm).
     * @return A CalibrationModel struct representing a valid EC calibration.
     */
    static CalibrationModel getSimulatedCalibrationModel();

    /**
     * @brief Provides a simulated raw ADC value for a mid-range EC (e.g., 1413 uS/cm).
     * This value corresponds to approximately 1.0V at the ADC input.
     * @return A simulated raw 16-bit integer ADC value.
     */
    static int16_t getSimulatedRawADC(); 

    // Future: Method to get simulated raw ADC value for a specific target EC value.
    // static int16_t getSimulatedRawADC(float target_ec_value);
};