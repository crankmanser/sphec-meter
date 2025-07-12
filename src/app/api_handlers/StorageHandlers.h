// src/app/api_handlers/StorageHandlers.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "managers/storage/StorageManager.h"
#include "config/Network_Config.h"

namespace Handlers {
    void handleStorageDiagnosticsStart(AsyncWebServerRequest *request, StorageManager* storage);
    void handleStorageDiagnosticsResult(AsyncWebServerRequest *request, StorageManager* storage);
    void handleBackup(AsyncWebServerRequest *request, StorageManager* storage);
    void handleSetConfig(AsyncWebServerRequest *request, StorageManager* storage, NetworkConfig* config);
    void handleRestoreUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, StorageManager* storage);
}