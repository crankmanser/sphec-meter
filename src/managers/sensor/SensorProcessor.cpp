// src/managers/sensor/SensorProcessor.cpp
#include "managers/sensor/SensorProcessor.h"
#include "DebugMacros.h"
#include "config/calibration_config.h" 
#include "config/calibration/pH_calibration_defaults.h"
#include "config/calibration/EC_calibration_defaults.h"
#include "config/calibration/filter_tuning_defaults.h"
#include <algorithm>
#include "managers/sensor/calibration/CalibrationSessionManager.h"

#include "config/DebugConfig.h"
#if (ENABLE_SENSOR_SIMULATION)
#include "debug/PhCalibrationSimulator.h"
#include "debug/EcCalibrationSimulator.h"
#endif

const float VOLTAGE_DIVIDER_COMPENSATION = 2.0f;
const float ADC_FSR_PROBES = 4.096f;

SensorProcessor::SensorProcessor(
    const RawSensorData* raw_data,
    ProcessedSensorData* processed_data,
    StorageManager& storage) :
        _raw_data(raw_data),
        _processed_data(processed_data),
        _storage(storage),
        _phFilter(new PIFilter(5, FilterTuningDefaults::DEFAULT_PH_FILTER_TUNING)),
        _ecFilter(new PIFilter(5, FilterTuningDefaults::DEFAULT_EC_FILTER_TUNING))
{
    // --- NEW: Instantiate the session manager ---
    _calibrationSessionManager = new CalibrationSessionManager(this);
}

void SensorProcessor::begin() {
    loadModels();
    loadFilterTuning();
    LOG_MANAGER("SensorProcessor initialized and models loaded.\n");
}

void SensorProcessor::update() {
    // ... (update logic remains the same)
    float ph_raw_voltage = adcValueToVoltage(_raw_data->adc_ph_raw) * VOLTAGE_DIVIDER_COMPENSATION;
    float ec_raw_voltage = adcValueToVoltage(_raw_data->adc_ec_raw) * VOLTAGE_DIVIDER_COMPENSATION;
    float ph_filtered_voltage = _phFilter->update(ph_raw_voltage);
    float ec_filtered_voltage = _ecFilter->update(ec_raw_voltage);
    float liquid_temp = _processed_data->liquid_temp_c;

    if (_ph_model.is_valid) {
        _processed_data->ph_value = CalibrationEngine::getCalibratedValue(_ph_model, CalibrationEngine::SensorType::PH, ph_filtered_voltage, liquid_temp);
    } else {
        _processed_data->ph_value = 0.0f;
    }

    if (_ec_model.is_valid) {
        _processed_data->ec_value = CalibrationEngine::getCalibratedValue(_ec_model, CalibrationEngine::SensorType::EC, ec_filtered_voltage, liquid_temp);
    } else {
        _processed_data->ec_value = 0.0f;
    }
}

// --- Public Calibration Management Methods (Delegation) ---

const CalibrationModel& SensorProcessor::getPhModel() const { return _ph_model; }
const CalibrationModel& SensorProcessor::getEcModel() const { return _ec_model; }

void SensorProcessor::startCalibration(CalibrationEngine::SensorType type) {
    _calibrationSessionManager->startCalibration(type);
}

bool SensorProcessor::captureCalibrationPoint(CalibrationEngine::SensorType sensor_type, CalibrationPointType point_type, float known_value) {
    return _calibrationSessionManager->captureCalibrationPoint(sensor_type, point_type, known_value);
}

bool SensorProcessor::finalizeCalibration(CalibrationEngine::SensorType type) {
    return _calibrationSessionManager->finalizeCalibration(type);
}

void SensorProcessor::cancelCalibration(CalibrationEngine::SensorType type) {
    _calibrationSessionManager->cancelCalibration(type);
}

bool SensorProcessor::isCalibrationActive(CalibrationEngine::SensorType type) const {
    return _calibrationSessionManager->isCalibrationActive(type);
}

SensorProcessor::CalibrationPointType SensorProcessor::getNextCalibrationPointType(CalibrationEngine::SensorType type) const {
    return _calibrationSessionManager->getNextCalibrationPointType(type);
}


// --- Internal calculation method, now called by the session manager ---
bool SensorProcessor::calculateNewModel(CalibrationEngine::SensorType type) {
    if (type == CalibrationEngine::SensorType::PH) {
        const auto& session = _calibrationSessionManager->getPhSession();
        CalibrationModel old_model = _ph_model;
        _ph_model = CalibrationEngine::calculateModel(session.points_voltage[0], session.points_value[0],
                                                    session.points_voltage[1], session.points_value[1],
                                                    session.points_voltage[2], session.points_value[2]);
        if (_ph_model.is_valid) {
            _ph_model.quality_score = CalibrationEngine::calculateQualityScore(_ph_model, CalibrationEngine::SensorType::PH);
            if (old_model.is_valid) _ph_model.last_sensor_drift = CalibrationEngine::calculateSensorDrift(_ph_model, old_model);
            savePhModel();
            LOG_MANAGER("PH calibration finalized and saved. Quality: %.1f%%\n", _ph_model.quality_score);
            return true;
        }
    } else { // EC
        const auto& session = _calibrationSessionManager->getEcSession();
        CalibrationModel old_model = _ec_model;
        _ec_model = CalibrationEngine::calculateModel(session.points_voltage[0], session.points_value[0],
                                                    session.points_voltage[1], session.points_value[1],
                                                    session.points_voltage[2], session.points_value[2]);
        if (_ec_model.is_valid) {
            _ec_model.quality_score = CalibrationEngine::calculateQualityScore(_ec_model, CalibrationEngine::SensorType::EC);
            if (old_model.is_valid) _ec_model.last_sensor_drift = CalibrationEngine::calculateSensorDrift(_ec_model, old_model);
            saveEcModel();
            LOG_MANAGER("EC calibration finalized and saved. Quality: %.1f%%\n", _ec_model.quality_score);
            return true;
        }
    }
    LOG_MAIN("[SP_ERROR] New model calculation failed.\n");
    return false;
}


void SensorProcessor::savePhModel() {
    // ... (logic remains the same)
}

void SensorProcessor::saveEcModel() {
    // ... (logic remains the same)
}

// ... (Other private methods and filter tuning methods remain the same)

float SensorProcessor::adcValueToVoltage(int16_t raw_value) {
    return (static_cast<float>(raw_value) / 32767.0f) * ADC_FSR_PROBES;
}

void SensorProcessor::loadModels() {
    // ... (logic remains the same)
}

void SensorProcessor::loadFilterTuning() {
    // ... (logic remains the same)
}

void SensorProcessor::saveFilterTuning() {
    // ... (logic remains the same)
}

float SensorProcessor::getFilteredPhVoltage() const { return _phFilter->getValue(); }
float SensorProcessor::getFilteredEcVoltage() const { return _ecFilter->getValue(); }
float SensorProcessor::getPhStability() const { return _phFilter->getStabilityFactor() * 100.0f; }
float SensorProcessor::getEcStability() const { return _ecFilter->getStabilityFactor() * 100.0f; }
const PIFilter::Tuni_t& SensorProcessor::getPhFilterTuning() const { return _filterTuningConfig.ph_tuning; }
const PIFilter::Tuni_t& SensorProcessor::getEcFilterTuning() const { return _filterTuningConfig.ec_tuning; }
void SensorProcessor::setPhFilterTuning(const PIFilter::Tuni_t& tuning) { _filterTuningConfig.ph_tuning = tuning; _phFilter->reconfigure(tuning); saveFilterTuning(); }
void SensorProcessor::setEcFilterTuning(const PIFilter::Tuni_t& tuning) { _filterTuningConfig.ec_tuning = tuning; _ecFilter->reconfigure(tuning); saveFilterTuning(); }