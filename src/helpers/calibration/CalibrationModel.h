#pragma once

#include <cstdint>

// This struct holds all data related to a single, complete sensor calibration.
// It includes the mathematical model, health KPIs, and metadata.
struct CalibrationModel {
    // --- The Mathematical Model (y = ax^2 + bx +c) ---
    // where 'y' is the scientific value and 'x' is the measured voltage.
    float a = 0.0f; // Quadratic coefficient
    float b = 0.0f; // Linear coefficient (slope)
    float c = 0.0f; // Offset

    // --- The Probe's "Fingerprint" ---
    // Raw voltage readings from the last good 3-point calibration.
    // Used for performing 1-point health checks.
    float low_point_v = 0.0f;
    float mid_point_v = 0.0f;
    float high_point_v = 0.0f;

    // --- Health & Drift KPIs ---
    bool is_valid = false;              // Is this model considered valid for use?
    uint32_t timestamp = 0;             // When this model was created
    float health_percent = 100.0f;      // Decaying health score of the probe
    float last_sensor_drift = 0.0f;   // Calculated drift vs. the previous model

    // --- Live Quality Score (Not Saved) ---
    // This is calculated live during a new calibration to score its quality.
    float quality_score = 0.0f;
};