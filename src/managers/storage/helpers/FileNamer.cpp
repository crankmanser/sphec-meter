// src/managers/storage/helpers/FileNamer.cpp
#include "FileNamer.h"

namespace FileNamer {
    const char* getFileName(ConfigType type) {
        switch (type) {
            case ConfigType::POWER_MANAGER_STATE: return "/power.state";
            case ConfigType::NETWORK_CONFIG: return "/network.conf";
            case ConfigType::LIQUID_TEMP_CONFIG: return "/liq_temp.conf";
            case ConfigType::AMBIENT_TEMP_CONFIG: return "/amb_temp.conf";
            case ConfigType::AMBIENT_HUMIDITY_CONFIG: return "/amb_hum.conf";
            case ConfigType::CALIBRATION_DATA: return "/calib.dat";
            case ConfigType::LDR_CONFIG: return "/ldr.conf";     
            case ConfigType::PH_CALIBRATION: return "/ph_cal.json";
            case ConfigType::EC_CALIBRATION: return "/ec_cal.json";
            case ConfigType::FILTER_TUNING_CONFIG: return "/filter_tuning.json";
            default: return "/unknown.state";
        }
    }

    const char* getTempFileName(ConfigType type) {
        switch (type) {
            case ConfigType::POWER_MANAGER_STATE: return "/power.tmp";
            case ConfigType::NETWORK_CONFIG: return "/network.tmp";
            case ConfigType::LIQUID_TEMP_CONFIG: return "/liq_temp.tmp";
            case ConfigType::AMBIENT_TEMP_CONFIG: return "/amb_temp.tmp";
            case ConfigType::AMBIENT_HUMIDITY_CONFIG: return "/amb_hum.tmp";
            case ConfigType::CALIBRATION_DATA: return "/calib.tmp";
            case ConfigType::LDR_CONFIG: return "/ldr.tmp";     
            case ConfigType::PH_CALIBRATION: return "/ph_cal.tmp";
            case ConfigType::EC_CALIBRATION: return "/ec_cal.tmp";
            case ConfigType::FILTER_TUNING_CONFIG: return "/filter_tuning.tmp";
            default: return "/unknown.tmp";
        }
    }
}