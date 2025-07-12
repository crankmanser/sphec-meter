// src/managers/sensor/calibration/CalibrationSessionManager.cpp
#include "CalibrationSessionManager.h"
#include "DebugMacros.h"

CalibrationSessionManager::CalibrationSessionManager(SensorProcessor* sensorProcessor) :
    _sensorProcessor(sensorProcessor)
{
    // Initialize calibration session states
    _ph_cal_session.active = false;
    _ph_cal_session.current_point_index = 0;
    _ec_cal_session.active = false;
    _ec_cal_session.current_point_index = 0;
}

void CalibrationSessionManager::startCalibration(CalibrationEngine::SensorType type) {
    if (type == CalibrationEngine::SensorType::PH) {
        _ph_cal_session.active = true;
        _ph_cal_session.current_point_index = 0;
        for (int i = 0; i < 3; ++i) {
            _ph_cal_session.points_voltage[i] = 0.0f;
            _ph_cal_session.points_value[i] = 0.0f;
        }
        LOG_MANAGER("PH calibration session started.\n");
    } else { // EC
        _ec_cal_session.active = true;
        _ec_cal_session.current_point_index = 0;
        for (int i = 0; i < 3; ++i) {
            _ec_cal_session.points_voltage[i] = 0.0f;
            _ec_cal_session.points_value[i] = 0.0f;
        }
        LOG_MANAGER("EC calibration session started.\n");
    }
}

bool CalibrationSessionManager::captureCalibrationPoint(CalibrationEngine::SensorType sensor_type, SensorProcessor::CalibrationPointType point_type, float known_value) {
    CalibrationSession* session = (sensor_type == CalibrationEngine::SensorType::PH) ? &_ph_cal_session : &_ec_cal_session;
    float current_filtered_voltage = (sensor_type == CalibrationEngine::SensorType::PH) ? _sensorProcessor->getFilteredPhVoltage() : _sensorProcessor->getFilteredEcVoltage();

    if (!session->active) {
        LOG_MAIN("[CSM_ERROR] Cannot capture point: calibration session not active.\n");
        return false;
    }
    if (session->current_point_index >= 3) {
        LOG_MAIN("[CSM_ERROR] Cannot capture point: all points already captured.\n");
        return false;
    }
    if (static_cast<uint8_t>(point_type) != session->current_point_index) {
        LOG_MAIN("[CSM_ERROR] Cannot capture point: unexpected point type. Expected %d, got %d.\n", session->current_point_index, static_cast<uint8_t>(point_type));
        return false;
    }

    session->points_voltage[session->current_point_index] = current_filtered_voltage;
    session->points_value[session->current_point_index] = known_value;
    LOG_MANAGER("Captured %s calibration point %d: Voltage=%.4fV, Value=%.2f\n",
                (sensor_type == CalibrationEngine::SensorType::PH) ? "PH" : "EC",
                session->current_point_index, current_filtered_voltage, known_value);
    session->current_point_index++;
    return true;
}

bool CalibrationSessionManager::finalizeCalibration(CalibrationEngine::SensorType type) {
    CalibrationSession* session = (type == CalibrationEngine::SensorType::PH) ? &_ph_cal_session : &_ec_cal_session;

    if (!session->active || session->current_point_index < 3) {
        LOG_MAIN("[CSM_ERROR] Cannot finalize calibration: Not enough points captured (need 3, have %d).\n", session->current_point_index);
        return false;
    }

    bool success = _sensorProcessor->calculateNewModel(type);
    if (success) {
        session->active = false; // End session on success
    }
    return success;
}

void CalibrationSessionManager::cancelCalibration(CalibrationEngine::SensorType type) {
    if (type == CalibrationEngine::SensorType::PH) {
        _ph_cal_session.active = false;
        _ph_cal_session.current_point_index = 0;
        LOG_MANAGER("PH calibration session cancelled.\n");
    } else { // EC
        _ec_cal_session.active = false;
        _ec_cal_session.current_point_index = 0;
        LOG_MANAGER("EC calibration session cancelled.\n");
    }
}

bool CalibrationSessionManager::isCalibrationActive(CalibrationEngine::SensorType type) const {
    return (type == CalibrationEngine::SensorType::PH) ? _ph_cal_session.active : _ec_cal_session.active;
}

SensorProcessor::CalibrationPointType CalibrationSessionManager::getNextCalibrationPointType(CalibrationEngine::SensorType type) const {
    const CalibrationSession* session = (type == CalibrationEngine::SensorType::PH) ? &_ph_cal_session : &_ec_cal_session;

    if (!session->active || session->current_point_index >= 3) {
        // <<< MODIFIED: Return type is now SensorProcessor::CalibrationPointType
        return static_cast<SensorProcessor::CalibrationPointType>(3);
    }
    return static_cast<SensorProcessor::CalibrationPointType>(session->current_point_index);
}

const CalibrationSessionManager::CalibrationSession& CalibrationSessionManager::getPhSession() const {
    return _ph_cal_session;
}

const CalibrationSessionManager::CalibrationSession& CalibrationSessionManager::getEcSession() const {
    return _ec_cal_session;
}