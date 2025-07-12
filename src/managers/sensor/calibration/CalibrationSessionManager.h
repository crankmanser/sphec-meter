// src/managers/sensor/calibration/CalibrationSessionManager.h
#pragma once

#include "helpers/calibration/CalibrationEngine.h"
// <<< MODIFIED: Include the full SensorProcessor header
#include "managers/sensor/SensorProcessor.h" 

// Forward declaration removed, full include is used now.

class CalibrationSessionManager {
public:
    // Struct to hold the state of a single calibration session
    struct CalibrationSession {
        float points_voltage[3];
        float points_value[3];
        uint8_t current_point_index;
        bool active;
    };

    CalibrationSessionManager(SensorProcessor* sensorProcessor);

    void startCalibration(CalibrationEngine::SensorType type);
    // <<< MODIFIED: Method signature uses the enum from SensorProcessor
    bool captureCalibrationPoint(CalibrationEngine::SensorType sensor_type, SensorProcessor::CalibrationPointType point_type, float known_value);
    bool finalizeCalibration(CalibrationEngine::SensorType type);
    void cancelCalibration(CalibrationEngine::SensorType type);

    bool isCalibrationActive(CalibrationEngine::SensorType type) const;
    // <<< MODIFIED: Method signature uses the enum from SensorProcessor
    SensorProcessor::CalibrationPointType getNextCalibrationPointType(CalibrationEngine::SensorType type) const;

    const CalibrationSession& getPhSession() const;
    const CalibrationSession& getEcSession() const;

private:
    SensorProcessor* _sensorProcessor;
    CalibrationSession _ph_cal_session;
    CalibrationSession _ec_cal_session;
};