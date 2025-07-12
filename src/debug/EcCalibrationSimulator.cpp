// src/debug/EcCalibrationSimulator.cpp
#include "EcCalibrationSimulator.h"
#include "config/calibration_config.h"             // For EC_CAL_POINT_LOW, etc.
#include "managers/sensor/SensorProcessor.h"              // For ADC_FSR_PROBES and VOLTAGE_DIVIDER_COMPENSATION constants
#include "DebugMacros.h"                           // For LOG_MANAGER

// Re-declare constants from SensorProcessor.cpp for use in simulation calculations
const float VOLTAGE_DIVIDER_COMPENSATION_SIM = 2.0f;
const float ADC_FSR_PROBES_SIM = 4.096f;

/**
 * @brief Provides a simulated, valid EC calibration model.
 * This model uses ideal voltage points for common EC buffers and calculates the quadratic coefficients.
 * @return A CalibrationModel struct representing a valid EC calibration.
 */
CalibrationModel EcCalibrationSimulator::getSimulatedCalibrationModel() {
    CalibrationModel model;

    // These are approximate voltage points for EC 84, 1413, and 12880 uS/cm
    // assuming a probe output that is then passed through the 10k+10k voltage divider.
    // The CalibrationEngine works with the pre-divider (compensated) voltage.
    float v_ec84_simulated = 0.1f;    // Represents voltage from EC probe itself for 84 uS/cm
    float v_ec1413_simulated = 1.0f;  // Represents voltage from EC probe itself for 1413 uS/cm
    float v_ec12880_simulated = 2.5f; // Represents voltage from EC probe itself for 12880 uS/cm

    model = CalibrationEngine::calculateModel(
        v_ec84_simulated, EC_CAL_POINT_LOW,
        v_ec1413_simulated, EC_CAL_POINT_MID,
        v_ec12880_simulated, EC_CAL_POINT_HIGH
    );

    if (model.is_valid) {
        model.quality_score = 100.0f; // Assume perfect quality for this predefined simulation
        model.last_sensor_drift = 0.0f; // No drift for a simulated model
        LOG_MANAGER("EcCalibrationSimulator: Generated a valid simulated calibration model.\n");
    } else {
        LOG_MAIN("[SIM_ERROR] EcCalibrationSimulator: Failed to generate simulated calibration model.\n");
    }
    return model;
}

/**
 * @brief Provides a simulated raw ADC value for a mid-range EC (e.g., 1413 uS/cm).
 * This value is chosen to correspond to ~1.0V *before* the voltage divider,
 * which means 0.5V at the ADC input.
 * @return A simulated raw 16-bit integer ADC value.
 */
int16_t EcCalibrationSimulator::getSimulatedRawADC() {
    // Target voltage at ADC input for a mid-range EC is ~0.5V,
    // assuming a 10k+10k voltage divider from a probe output of 1.0V.
    const float target_adc_voltage = 0.5f; // Voltage AT THE ADC PIN

    // Calculate the raw ADC value from the target voltage
    // Raw ADC Value = (Target Voltage / ADC_FSR_PROBES) * 32767.0f
    int16_t raw_adc_value = static_cast<int16_t>((target_adc_voltage / ADC_FSR_PROBES_SIM) * 32767.0f);
    
    // Log the simulated value
    LOG_MANAGER("EcCalibrationSimulator: Providing simulated raw ADC for EC: %d (approx 1413 uS/cm)\n", raw_adc_value);

    return raw_adc_value;
}