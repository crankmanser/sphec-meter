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
 * @brief --- DEFINITIVE UPDATE: Overloaded to handle both working and saved states. ---
 */
bool ConfigManager::saveFilterSettings(FilterManager& filter, const char* filterName, bool is_saved_state) {
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

    char filepath[64];
    if (is_saved_state) {
        snprintf(filepath, sizeof(filepath), "/config/%s_saved.json", filterName);
    } else {
        snprintf(filepath, sizeof(filepath), "/config/%s.json", filterName);
    }

    LOG_STORAGE("Saving filter settings to %s", filepath);
    return _sdManager->saveJson(filepath, doc);
}

/**
 * @brief --- DEFINITIVE UPDATE: Overloaded to handle both working and saved states. ---
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
        LOG_STORAGE("Failed to load %s.", filepath);
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