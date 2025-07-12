// src/managers/storage/StorageManager_types.h
#pragma once
#include <cstdint>

enum class ConfigType {
    POWER_MANAGER_STATE,
    NETWORK_CONFIG,
    CALIBRATION_DATA,
    LIQUID_TEMP_CONFIG,
    AMBIENT_TEMP_CONFIG,
    AMBIENT_HUMIDITY_CONFIG,
    LDR_CONFIG,
    PH_CALIBRATION, 
    EC_CALIBRATION,  
    FILTER_TUNING_CONFIG 
};

// <<< MODIFIED: The diagnostic result struct now lives here.
// This struct holds the results of the diagnostic test.
struct StorageDiagnosticResult {
    bool is_sd_card_initialized = false;
    bool can_create_file = false;
    bool can_write_file = false;
    bool can_read_file = false;
    bool can_delete_file = false;
    bool is_passed = false;
    uint32_t free_space_kb = 0;
    uint32_t timestamp = 0; // To know when the test was run
};