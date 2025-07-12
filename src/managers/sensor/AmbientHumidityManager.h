#pragma once

#include "data_models/SensorData_types.h"
#include "managers/storage/StorageManager.h"

// This struct holds the configuration for this manager.
struct AmbientHumidityCalConfig {
    float dht11_offset_percent = 0.0f;
};

class AmbientHumidityManager {
public:
    AmbientHumidityManager(
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
    AmbientHumidityCalConfig _config;
};