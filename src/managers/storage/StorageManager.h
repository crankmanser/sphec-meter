// src/managers/storage/StorageManager.h
// MODIFIED FILE
#pragma once

#include <Arduino.h>
#include <SdFat.h>
#include <SPI.h>
#include <vector>
#include "StorageManager_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

class StorageDiagnostics;

// <<< MODIFIED: Added RECOVER operation type >>>
enum class FileOperationType {
    WRITE,
    DIAGNOSTICS,
    RECOVER
};

struct FileOperationRequest {
    FileOperationType op_type;
    ConfigType config_type;
    std::vector<uint8_t> data;
};

class StorageManager {
public:
    StorageManager(uint8_t cs_pin, SPIClass* spi, SemaphoreHandle_t spi_bus_mutex, SemaphoreHandle_t diag_result_mutex);
    ~StorageManager();

    bool begin();

    // --- State & Config I/O ---
    bool saveState(ConfigType type, const uint8_t* data, size_t len);
    bool loadState(ConfigType type, uint8_t* data, size_t len);
    std::vector<uint8_t> readFile(ConfigType type);

    // --- Backup & Restore ---
    bool restoreBackup(const std::vector<uint8_t>& backupData);

    // --- System Integrity ---
    bool requestRecovery(); // <<< MODIFIED: This is now the public method
    bool writeShutdownFlag();
    bool checkAndClearShutdownFlag();

    // --- Diagnostics ---
    bool requestDiagnostics();
    StorageDiagnosticResult getDiagnosticResult() const;

    // --- RTOS Members ---
    static TaskHandle_t _storageTaskHandle;
    static QueueHandle_t _fileQueue;

private:
    uint8_t _cs_pin;
    SPIClass* _spi;
    SdFs _sd;
    SemaphoreHandle_t _spi_bus_mutex;
    SemaphoreHandle_t _diag_result_mutex;
    bool _is_ready = false;

    StorageDiagnostics* _diagnostics;
    volatile StorageDiagnosticResult _last_diag_result;

    void recoverFromCrash(); // <<< MODIFIED: Now a private helper
    void runAndStoreDiagnostics();
    static void storageTask(void* pvParameters);
};