// File Path: /lib/ConfigManager/src/ConfigManager.cpp
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
    if (strcmp(sessionTimestamp, "default") == 0) {
         snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
         LOG_STORAGE("Saving operational filter settings to %s", filepath);
    }
    else {
        snprintf(filepath, sizeof(filepath), "/config/%s_log_%s.json", filterName, sessionTimestamp);
        LOG_STORAGE("Saving timestamped filter log to %s", filepath);
    }

    return _sdManager->saveJson(filepath, doc);
}


/**
 * @brief --- DEFINITIVE REFACTOR: This function no longer performs writes. ---
 * To resolve the initialization race condition, this function's responsibility
 * has been simplified. It now ONLY attempts to load a file. If the file
 * doesn't exist, it returns false. The responsibility of creating default
 * files has been moved to a centralized "provisioner" in main.cpp.
 */
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
        LOG_STORAGE("File not found: %s", filepath);
        // Do NOT create a default file here. This is the key to the fix.
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
        lfFilter->trackAssist = lfFilter->trackAssist;
        lfFilter->medianWindowSize = lf["medianWindowSize"];
    }

    LOG_STORAGE("Successfully loaded settings from %s", filepath);
    return true;
}