#include "managers/sensor/LiquidTempManager.h"
#include "DebugMacros.h"

LiquidTempManager::LiquidTempManager(
    const RawSensorData* raw_data,
    ProcessedSensorData* processed_data,
    StorageManager& storage) :
        _raw_data(raw_data),
        _processed_data(processed_data),
        _storage(storage)
{}

void LiquidTempManager::begin() {
    loadConfig();
    LOG_MANAGER("LiquidTempManager initialized.\n");
}

void LiquidTempManager::update() {
    // <<< MODIFIED: All mutex calls are REMOVED.
    
    float raw_temp = _raw_data->temp_ds18b20_raw;

    if (isnan(raw_temp)) {
        return; 
    }
    
    float calibrated_temp = raw_temp + _config.ds18b20_offset_c;

    _processed_data->liquid_temp_c = calibrated_temp;
}

void LiquidTempManager::loadConfig() {
    // We will use a new ConfigType for this manager's settings.
    // Note: We need to add `LIQUID_TEMP_CONFIG` to the StorageManager_types.h enum.
    if (!_storage.loadState(ConfigType::LIQUID_TEMP_CONFIG, (uint8_t*)&_config, sizeof(_config))) {
        LOG_MANAGER("No liquid temp config found. Using defaults.\n");
        // Default offset is 0.0, which is the default for the struct.
        saveConfig();
    }
}

void LiquidTempManager::saveConfig() {
    if (!_storage.saveState(ConfigType::LIQUID_TEMP_CONFIG, (const uint8_t*)&_config, sizeof(_config))) {
        LOG_MAIN("[LTM_ERROR] Failed to save liquid temp config!\n");
    }
}