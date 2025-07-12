#pragma once

#include "data_models/SensorData_types.h"
#include "managers/storage/StorageManager.h"

// This struct will hold the configuration for this manager,
// such as calibration points for voltage-to-percentage conversion.
struct LdrConfig {
    float min_voltage = 0.5f; // Voltage in complete darkness
    float max_voltage = 3.0f; // Voltage in bright light
};

class LDRManager {
public:
    LDRManager(
        const RawSensorData* raw_data,
        ProcessedSensorData* processed_data,
        StorageManager& storage
    );

    void begin();
    void update();

private:
    void loadConfig();
    void saveConfig();

    const RawSensorData* _raw_data;
    ProcessedSensorData* _processed_data;
    StorageManager& _storage;
    LdrConfig _config;
};