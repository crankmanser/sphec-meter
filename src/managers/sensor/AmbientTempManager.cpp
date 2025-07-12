#include "managers/sensor/AmbientTempManager.h"
#include "DebugMacros.h"

AmbientTempManager::AmbientTempManager(
    const RawSensorData* raw_data,
    ProcessedSensorData* processed_data,
    StorageManager& storage) :
        _raw_data(raw_data),
        _processed_data(processed_data),
        _storage(storage)
{}

void AmbientTempManager::begin() {
    loadConfig();
    LOG_MANAGER("AmbientTempManager initialized.\n");
}

void AmbientTempManager::update() {
    float raw_temp = _raw_data->temp_dht11_raw;

    if (isnan(raw_temp)) {
        return; 
    }
    
    float calibrated_temp = raw_temp + _config.dht11_offset_c;
    
    _processed_data->ambient_temp_c = calibrated_temp;
}

void AmbientTempManager::loadConfig() {
    if (!_storage.loadState(ConfigType::AMBIENT_TEMP_CONFIG, (uint8_t*)&_config, sizeof(_config))) {
        LOG_MANAGER("No ambient temp config found. Using defaults.\n");
        saveConfig();
    }
}

void AmbientTempManager::saveConfig() {
    if (!_storage.saveState(ConfigType::AMBIENT_TEMP_CONFIG, (const uint8_t*)&_config, sizeof(_config))) {
        LOG_MAIN("[ATM_ERROR] Failed to save ambient temp config!\n");
    }
}