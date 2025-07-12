#include "managers/sensor/LDRManager.h"
#include "DebugMacros.h"

LDRManager::LDRManager(
    const RawSensorData* raw_data,
    ProcessedSensorData* processed_data,
    StorageManager& storage) :
        _raw_data(raw_data),
        _processed_data(processed_data),
        _storage(storage)
{}

void LDRManager::begin() {
    loadConfig();
    LOG_MANAGER("LDRManager initialized.\n");
}

void LDRManager::update() {
    float raw_voltage = _raw_data->ldr_voltage_raw;

    if (isnan(raw_voltage)) {
        _processed_data->light_level_percent = 0.0f; // Default to 0 on error
        return;
    }

    // Map the raw voltage to a 0-100% range using the stored calibration
    float light_level = 100.0f * (raw_voltage - _config.min_voltage) / (_config.max_voltage - _config.min_voltage);

    // Constrain the value to a valid percentage range
    _processed_data->light_level_percent = constrain(light_level, 0.0f, 100.0f);
}

void LDRManager::loadConfig() {
    // Note: We need to add LDR_CONFIG to the StorageManager_types.h enum
    if (!_storage.loadState(ConfigType::LDR_CONFIG, (uint8_t*)&_config, sizeof(_config))) {
        LOG_MANAGER("No LDR config found. Using defaults.\n");
        saveConfig();
    }
}

void LDRManager::saveConfig() {
    if (!_storage.saveState(ConfigType::LDR_CONFIG, (const uint8_t*)&_config, sizeof(_config))) {
        LOG_MAIN("[LDR_ERROR] Failed to save LDR config!\n");
    }
}