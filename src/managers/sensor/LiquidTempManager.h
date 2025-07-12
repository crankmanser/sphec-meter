#pragma once

#include "data_models/SensorData_types.h"
#include "managers/storage/StorageManager.h"


// This struct will hold the configuration for this manager,
// specifically the user-defined calibration offset.
struct TempCalConfig {
    float ds18b20_offset_c = 0.0f;
};

class LiquidTempManager {
public:
    // The constructor takes pointers to the shared data structs, their mutexes,
    // and a reference to the StorageManager for persistence.
    LiquidTempManager(
        const RawSensorData* raw_data,
        ProcessedSensorData* processed_data,
        StorageManager& storage
    );

    void begin();

    // This single method reads the raw data, applies the offset,
    // and updates the processed data struct.
    void update();

    // Future methods for UI interaction
    // void setCalibrationOffset(float offset);
    // void saveConfig();

private:
    void loadConfig();
    void saveConfig();

    // Pointers to the shared data
    const RawSensorData* _raw_data;
    ProcessedSensorData* _processed_data;

    // A reference to the storage service
    StorageManager& _storage;
    
    // The manager's internal configuration state
    TempCalConfig _config;
};