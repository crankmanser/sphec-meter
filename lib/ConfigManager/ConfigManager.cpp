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
        // Ensure the base config directory exists
        _sdManager->mkdir("/config");
    }
    return true;
}

bool ConfigManager::saveFilterSettings(FilterManager& filter, const char* filterName, const char* sessionTimestamp, bool is_saved_state) {
    if (!_initialized || !_sdManager) return false;

    // 1. Serialize the filter settings into a JSON document
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

    // 2. Construct a unique filename to prevent overwrites
    char filepath[128];
    // If timestamp is "default", save to the main config file.
    if (strcmp(sessionTimestamp, "default") == 0) {
         snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
    } else {
        int counter = 0;
        do {
            const char* saved_suffix = is_saved_state ? "_saved" : "";
            if (counter == 0) {
                snprintf(filepath, sizeof(filepath), "/config/%s%s_%s.json", filterName, saved_suffix, sessionTimestamp);
            } else {
                snprintf(filepath, sizeof(filepath), "/config/%s%s_%s_%d.json", filterName, saved_suffix, sessionTimestamp, counter);
            }
            counter++;
        } while (_sdManager->open(filepath, FILE_READ));
    }

    LOG_STORAGE("Saving unique filter settings to %s", filepath);
    
    // 3. Save the JSON document to the unique file
    return _sdManager->saveJson(filepath, doc);
}


bool ConfigManager::loadFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state) {
    if (!_initialized || !_sdManager) return false;

    char filepath[64];
    if (is_saved_state) {
        snprintf(filepath, sizeof(filepath), "/config/%s_saved.json", filterName);
    } else {
        snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
    }

    StaticJsonDocument<512> doc;
    LOG_STORAGE("Loading filter settings from %s", filepath);
    if (!_sdManager->loadJson(filepath, doc)) {
        LOG_STORAGE("Failed to load %s. Creating a new default.", filepath);
        saveFilterSettings(filter, filterName, "default", is_saved_state);
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
        // --- DEFINITIVE FIX: Corrected the typo in the variable name ---
        lfFilter->medianWindowSize = lf["medianWindowSize"];
    }
    
    LOG_STORAGE("Successfully loaded settings from %s", filepath);
    return true;
}