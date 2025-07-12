#include "PhCalibrationSimulator.h"
#include "config/calibration_config.h"             // For PH_CAL_POINT_LOW, PH_CAL_POINT_MID, PH_CAL_POINT_HIGH
#include "managers/sensor/SensorProcessor.h"              // For ADC_FSR_PROBES and VOLTAGE_DIVIDER_COMPENSATION constants
#include "DebugMacros.h"                           // For LOG_MANAGER

// Re-declare constants from SensorProcessor.cpp for use in simulation calculations
// These should ideally be in a shared config or passed, but for debug utility, direct use is acceptable.
const float VOLTAGE_DIVIDER_COMPENSATION = 2.0f;
const float ADC_FSR_PROBES = 4.096f;

/**
 * @brief Provides a simulated, valid pH calibration model.
 * This model uses ideal voltage points for common pH buffers (pH 4.01, 6.86, 9.18)
 * and calculates the quadratic coefficients.
 * @return A CalibrationModel struct representing a valid pH calibration.
 */
CalibrationModel PhCalibrationSimulator::getSimulatedCalibrationModel() {
    CalibrationModel model;

    // These are approximate voltage points for pH 4.01, 6.86, 9.18
    // assuming a 0-5V swing where pH 7.0 is ~2.5V, and compensated for a 10k+10k voltage divider.
    // So, actual voltage *at ADC input* would be 3.0V / 2.0 = 1.5V (for pH 4.01)
    // and 2.5V / 2.0 = 1.25V (for pH 6.86), etc.
    // However, the CalibrationEngine takes the *actual* voltage sensed by the probe after compensation,
    // so we use the pre-divider values.
    float v_ph4_simulated = 3.0f; // Represents voltage from pH probe itself for 4.01
    float v_ph7_simulated = 2.5f; // Represents voltage from pH probe itself for 6.86
    float v_ph9_simulated = 2.0f; // Represents voltage from pH probe itself for 9.18

    model = CalibrationEngine::calculateModel(
        v_ph4_simulated, PH_CAL_POINT_LOW,
        v_ph7_simulated, PH_CAL_POINT_MID,
        v_ph9_simulated, PH_CAL_POINT_HIGH
    );

    if (model.is_valid) {
        model.quality_score = 100.0f; // Assume perfect quality for this predefined simulation
        model.last_sensor_drift = 0.0f; // No drift for a simulated model
        LOG_MANAGER("PhCalibrationSimulator: Generated a valid simulated calibration model.\n");
    } else {
        LOG_MAIN("[SIM_ERROR] PhCalibrationSimulator: Failed to generate simulated calibration model.\n");
    }
    return model;
}

/**
 * @brief Provides a simulated raw ADC value for a neutral pH (e.g., pH 7.0).
 * This value is chosen to correspond to ~2.5V after the voltage divider,
 * or 1.25V at the ADC input given the voltage divider.
 * @return A simulated raw 16-bit integer ADC value.
 */
int16_t PhCalibrationSimulator::getSimulatedRawADC() {
    // Target voltage at ADC input for a neutral pH (pH 6.86 - 7.0) is typically around 1.25V,
    // assuming a 10k+10k voltage divider from a probe output that centers around 2.5V.
    const float target_adc_voltage = 1.25f; // Voltage at the ADC pin itself

    // Calculate the raw ADC value from the target voltage
    // Raw ADC Value = (Target Voltage / ADC_FSR_PROBES) * 32767.0f
    int16_t raw_adc_value = static_cast<int16_t>((target_adc_voltage / ADC_FSR_PROBES) * 32767.0f);
    
    // Log the simulated value, ensuring it's clear this is not a real reading
    LOG_MANAGER("PhCalibrationSimulator: Providing simulated raw ADC for pH: %d (approx pH 7.0)\n", raw_adc_value);

    return raw_adc_value;
}