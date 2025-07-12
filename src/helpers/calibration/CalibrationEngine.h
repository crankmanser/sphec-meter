// src/helpers/calibration/CalibrationEngine.h
#pragma once

#include <cmath>
#include <Arduino.h> // <<< ADDED: Include Arduino framework for standard functions like constrain()
#include "helpers/calibration/CalibrationModel.h"

namespace CalibrationEngine {

    // An enum to specify the type of sensor for temperature compensation.
    enum class SensorType {
        PH,
        EC
    };

    // Performs a quadratic regression on three calibration points (x=voltage, y=scientific_value).
    // Returns a new CalibrationModel with the calculated coefficients (a, b, c).
    inline CalibrationModel calculateModel(
        float x1, float y1,
        float x2, float y2,
        float x3, float y3)
    {
        CalibrationModel model;

        // Standard formulas for quadratic regression
        float d = (x1 - x2) * (x1 - x3) * (x2 - x3);
        if (std::abs(d) < 1e-6) { // Avoid division by zero
            model.is_valid = false;
            return model;
        }

        model.a = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / d;
        model.b = ((y1 - y2) * (x3*x3 - x1*x1) - (y1 - y3) * (x2*x2 - x1*x1)) / d;
        model.c = y1 - model.a * x1 * x1 - model.b * x1;
        
        model.is_valid = true;
        model.low_point_v = x1;
        model.mid_point_v = x2;
        model.high_point_v = x3;

        return model;
    }

    // Applies the quadratic model and temperature compensation to get a final value.
    inline float getCalibratedValue(
        const CalibrationModel& model,
        SensorType type,
        float voltage,
        float temperature_c)
    {
        if (!model.is_valid) {
            return NAN;
        }

        // Base calculation using the y = ax^2 + bx + c model
        float value = model.a * voltage * voltage + model.b * voltage + model.c;

        // Apply scientific temperature compensation
        if (type == SensorType::PH) {
            // Nernst equation compensation for pH
            float nernst_slope = 198.41f * (temperature_c + 273.15f);
            value += (7.0f - value) * (1.0f - (298.15f / (temperature_c + 273.15f)));
        } else { // EC
            // Standard linear temperature compensation for EC
            const float ec_temp_coeff = 0.019f; // Typical for KCl solutions
            value /= (1.0f + ec_temp_coeff * (temperature_c - 25.0f));
        }

        return value;
    }

    // Calculates the "goodness-of-fit" for a new calibration, returning a score from 0-100.
    inline float calculateQualityScore(const CalibrationModel& new_model, SensorType type) {
        if (!new_model.is_valid) return 0.0f;

        float slope_at_mid = 2 * new_model.a * new_model.mid_point_v + new_model.b;
        
        // Define ideal slopes (mV per unit of pH/EC)
        float ideal_slope = (type == SensorType::PH) ? -59.16f : 1.0f; // Simplified for EC
        
        // Score is based on how close the calculated slope is to the ideal slope.
        float slope_accuracy = 1.0f - std::abs((slope_at_mid - ideal_slope) / ideal_slope);
        
        return constrain(slope_accuracy * 100.0f, 0.0f, 100.0f);
    }

    // Compares a new calibration model to an old one to quantify the probe's drift.
    inline float calculateSensorDrift(const CalibrationModel& new_model, const CalibrationModel& old_model) {
        if (!new_model.is_valid || !old_model.is_valid) return 0.0f;

        // Calculate the area difference between the two quadratic curves over a fixed voltage range.
        auto integral = [&](const CalibrationModel& m, float v_start, float v_end) {
            return (m.a/3.0f * (v_end*v_end*v_end - v_start*v_start*v_start) +
                    m.b/2.0f * (v_end*v_end - v_start*v_start) +
                    m.c * (v_end - v_start));
        };

        float voltage_range_start = 0.0f;
        float voltage_range_end = 4.096f;

        float old_area = integral(old_model, voltage_range_start, voltage_range_end);
        float new_area = integral(new_model, voltage_range_start, voltage_range_end);

        if (std::abs(old_area) < 1e-6) return 0.0f;

        return std::abs((new_area - old_area) / old_area) * 100.0f; // Return drift as a percentage
    }
}