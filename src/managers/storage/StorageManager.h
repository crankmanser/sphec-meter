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

enum class FileOperationType {
    WRITE,
    DIAGNOSTICS,
    RECOVER,
    CHECK_DEFAULTS 
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
    void startRtosDependencies();

    // --- State & Config I/O ---
    bool saveState(ConfigType type, const uint8_t* data, size_t len);
    bool loadState(ConfigType type, uint8_t* data, size_t len);
    std::vector<uint8_t> readFile(ConfigType type);

    // --- Backup & Restore ---
    bool restoreBackup(const std::vector<uint8_t>& backupData);

    // --- System Integrity ---
    bool requestRecovery();
    bool writeShutdownFlag();
    bool checkAndClearShutdownFlag();

    // --- Diagnostics ---
    bool requestDiagnostics();
    StorageDiagnosticResult getDiagnosticResult() const;

    // <<< ADDED: Public getter for the underlying SdFs object >>>
    // This allows other parts of the system (like our main.cpp test loop)
    // to perform direct file operations when necessary.
    SdFs* getSdFs();

private:
    uint8_t _cs_pin;
    SPIClass* _spi;
    SdFs _sd; // This is the actual SdFat file system object
    SemaphoreHandle_t _spi_bus_mutex;
    SemaphoreHandle_t _diag_result_mutex;
    bool _is_ready = false;

    StorageDiagnostics* _diagnostics;
    StorageDiagnosticResult _last_diag_result;

    TaskHandle_t _storageTaskHandle;
    QueueHandle_t _fileQueue;
    bool _rtos_initialized; 

    void checkAndCreateDefaults();
    void recoverFromCrash();
    void runAndStoreDiagnostics();
    static void storageTask(void* pvParameters);
};