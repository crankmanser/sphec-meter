#pragma once

#include "data_models/SensorData_types.h"
#include "managers/storage/StorageManager.h"

// This struct holds the configuration for this manager.
struct AmbientTempCalConfig {
    float dht11_offset_c = 0.0f;
};

class AmbientTempManager {
public:
    // <<< MODIFIED: The constructor no longer knows about mutexes.
    AmbientTempManager(
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
    AmbientTempCalConfig _config;
};