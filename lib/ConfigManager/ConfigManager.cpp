// File Path: /lib/ConfigManager/ConfigManager.cpp

#include "ConfigManager.h"

//
// --- CONSTRUCTOR ---
//
ConfigManager::ConfigManager() : _faultHandler(nullptr), _initialized(false) {
    // Constructor intentionally left empty.
    // Dependencies are injected via the begin() method.
}

//
// --- INITIALIZATION ---
//
bool ConfigManager::begin(FaultHandler& faultHandler) {
    _faultHandler = &faultHandler;

    // In the future, this is where we would initialize the NVS (Non-Volatile Storage)
    // and verify that we can communicate with the StorageEngine.

    _initialized = true;
    return true; // Return true assuming initialization is successful for now.
}