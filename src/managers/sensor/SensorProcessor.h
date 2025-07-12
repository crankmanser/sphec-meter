// src/managers/sensor/SensorProcessor.h
#pragma once

#include "data_models/SensorData_types.h"
#include "managers/storage/StorageManager.h"
#include "helpers/calibration/CalibrationModel.h"
#include "helpers/calibration/CalibrationEngine.h"
#include "helpers/filters/PIFilter.h"

// Forward-declare to avoid circular dependencies
class CalibrationSessionManager;

// Struct for Filter Tuning Configuration (for persistence)
struct FilterTuningConfig {
    PIFilter::Tuni_t ph_tuning;
    PIFilter::Tuni_t ec_tuning;
    bool is_valid = false;
};

class SensorProcessor {
public:
    // <<< MODIFIED: The enum now lives here, making it accessible to all consumers.
    enum class CalibrationPointType {
        POINT_LOW,
        POINT_MID,
        POINT_HIGH
    };

    SensorProcessor(
        const RawSensorData* raw_data,
        ProcessedSensorData* processed_data,
        StorageManager& storage
    );

    void begin();
    void update();

    const CalibrationModel& getPhModel() const;
    const CalibrationModel& getEcModel() const;

    // --- Public methods that now delegate to CalibrationSessionManager ---
    void startCalibration(CalibrationEngine::SensorType type);
    // <<< MODIFIED: Method signature now correctly uses the local enum
    bool captureCalibrationPoint(CalibrationEngine::SensorType sensor_type, CalibrationPointType point_type, float known_value);
    bool finalizeCalibration(CalibrationEngine::SensorType type);
    void cancelCalibration(CalibrationEngine::SensorType type);
    bool isCalibrationActive(CalibrationEngine::SensorType type) const;
    CalibrationPointType getNextCalibrationPointType(CalibrationEngine::SensorType type) const;

    // --- Internal calculation methods, made public for the session manager ---
    bool calculateNewModel(CalibrationEngine::SensorType type);
    void savePhModel();
    void saveEcModel();

    float getFilteredPhVoltage() const;
    float getFilteredEcVoltage() const;
    float getPhStability() const;
    float getEcStability() const;

    // Public methods for Filter Tuning
    const PIFilter::Tuni_t& getPhFilterTuning() const;
    const PIFilter::Tuni_t& getEcFilterTuning() const;
    void setPhFilterTuning(const PIFilter::Tuni_t& tuning);
    void setEcFilterTuning(const PIFilter::Tuni_t& tuning);
    void saveFilterTuning();

private:
    const RawSensorData* _raw_data;
    ProcessedSensorData* _processed_data;
    StorageManager& _storage;

    CalibrationModel _ph_model;
    CalibrationModel _ec_model;

    PIFilter* _phFilter;
    PIFilter* _ecFilter;

    FilterTuningConfig _filterTuningConfig;

    // Pointer to the dedicated session manager
    CalibrationSessionManager* _calibrationSessionManager;

    float adcValueToVoltage(int16_t raw_value);
    
    void loadModels();
    void loadFilterTuning();
};