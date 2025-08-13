// File Path: /lib/ConfigManager/ConfigManager.cpp
// MODIFIED FILE

#include "ConfigManager.h"
#include "../../src/DebugConfig.h"

ConfigManager::ConfigManager() : 
    _faultHandler(nullptr), 
    _sdManager(nullptr),
    _initialized(false) 
{}

bool ConfigManager::begin(FaultHandler& faultHandler, SdManager& sdManager) {
    _faultHandler = &faultHandler;
    _sdManager = &sdManager;
    _initialized = true;
    if (_sdManager) {
        _sdManager->mkdir("/config");
    }
    return true;
}

/**
 * @brief --- DEFINITIVE REFACTOR: Implements the robust, dual-save logic. ---
 * This function now correctly handles saving both primary operational files and
 * timestamped log files based on the provided sessionTimestamp.
 */
bool ConfigManager::saveFilterSettings(FilterManager& filter, const char* filterName, const char* sessionTimestamp, bool is_saved_state) {
    if (!_initialized || !_sdManager) return false;

    StaticJsonDocument<512> doc;
    PI_Filter* hfFilter = filter.getFilter(0);
    if (hfFilter) {
        JsonObject hf = doc.createNestedObject("hf_filter");
        hf["settleThreshold"] = hfFilter->settleThreshold;
        hf["lockSmoothing"] = hfFilter->lockSmoothing;
        hf["trackResponse"] = hfFilter->trackResponse;
        hf["trackAssist"] = hfFilter->trackAssist;
        hf["medianWindowSize"] = hfFilter->medianWindowSize;
    }
    PI_Filter* lfFilter = filter.getFilter(1);
    if (lfFilter) {
        JsonObject lf = doc.createNestedObject("lf_filter");
        lf["settleThreshold"] = lfFilter->settleThreshold;
        lf["lockSmoothing"] = lfFilter->lockSmoothing;
        lf["trackResponse"] = lfFilter->trackResponse;
        lf["trackAssist"] = lfFilter->trackAssist;
        lf["medianWindowSize"] = lfFilter->medianWindowSize;
    }

    char filepath[128];
    // If timestamp is "default", save to the main operational config file.
    if (strcmp(sessionTimestamp, "default") == 0) {
         snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
         LOG_STORAGE("Saving operational filter settings to %s", filepath);
    } 
    // Otherwise, create a unique, timestamped log file.
    else {
        snprintf(filepath, sizeof(filepath), "/config/%s_log_%s.json", filterName, sessionTimestamp);
        LOG_STORAGE("Saving timestamped filter log to %s", filepath);
    }
    
    return _sdManager->saveJson(filepath, doc);
}


bool ConfigManager::loadFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state) {
    if (!_initialized || !_sdManager) return false;

    char filepath[64];
    // The "is_saved_state" parameter is now effectively a way to choose which file to load.
    // True = load the user's saved tune. False = load the system's default startup tune.
    if (is_saved_state) {
        snprintf(filepath, sizeof(filepath), "/config/%s_saved.json", filterName);
    } else {
        snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
    }

    StaticJsonDocument<512> doc;
    LOG_STORAGE("Loading filter settings from %s", filepath);
    if (!_sdManager->loadJson(filepath, doc)) {
        LOG_STORAGE("Failed to load %s. Creating a new default.", filepath);
        // This call now correctly saves a file named "filterName.json"
        saveFilterSettings(filter, filterName, "default", false);
        return false;
    }

    PI_Filter* hfFilter = filter.getFilter(0);
    JsonObject hf = doc["hf_filter"];
    if (hfFilter && !hf.isNull()) {
        hfFilter->settleThreshold = hf["settleThreshold"];
        hfFilter->lockSmoothing = hf["lockSmoothing"];
        hfFilter->trackResponse = hf["trackResponse"];
        hfFilter->trackAssist = hf["trackAssist"];
        hfFilter->medianWindowSize = hf["medianWindowSize"];
    }

    PI_Filter* lfFilter = filter.getFilter(1);
    JsonObject lf = doc["lf_filter"];
    if (lfFilter && !lf.isNull()) {
        lfFilter->settleThreshold = lf["settleThreshold"];
        lfFilter->lockSmoothing = lf["lockSmoothing"];
        lfFilter->trackResponse = lf["trackResponse"];
        lfFilter->trackAssist = lf["trackAssist"];
        lfFilter->medianWindowSize = lf["medianWindowSize"];
    }
    
    LOG_STORAGE("Successfully loaded settings from %s", filepath);
    return true;
}