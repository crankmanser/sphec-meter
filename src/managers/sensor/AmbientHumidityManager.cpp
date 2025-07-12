#include "managers/sensor/AmbientHumidityManager.h"
#include "DebugMacros.h"

// Corrected constructor name
AmbientHumidityManager::AmbientHumidityManager(
    const RawSensorData* raw_data,
    ProcessedSensorData* processed_data,
    StorageManager& storage) :
        _raw_data(raw_data),
        _processed_data(processed_data),
        _storage(storage)
{}

// Corrected method name
void AmbientHumidityManager::begin() {
    loadConfig();
    LOG_MANAGER("AmbientHumidityManager initialized.\n");
}

// Corrected method name
void AmbientHumidityManager::update() {
    float raw_humidity = _raw_data->humidity_dht11_raw;

    if (isnan(raw_humidity)) {
        return; 
    }
    
    float calibrated_humidity = raw_humidity + _config.dht11_offset_percent;
    
    _processed_data->ambient_humidity_percent = calibrated_humidity;
}

// Corrected method name
void AmbientHumidityManager::loadConfig() {
    if (!_storage.loadState(ConfigType::AMBIENT_HUMIDITY_CONFIG, (uint8_t*)&_config, sizeof(_config))) {
        LOG_MANAGER("No ambient humidity config found. Using defaults.\n");
        saveConfig();
    }
}

// Corrected method name
void AmbientHumidityManager::saveConfig() {
    if (!_storage.saveState(ConfigType::AMBIENT_HUMIDITY_CONFIG, (const uint8_t*)&_config, sizeof(_config))) {
        LOG_MAIN("[AHM_ERROR] Failed to save ambient humidity config!\n");
    }
}