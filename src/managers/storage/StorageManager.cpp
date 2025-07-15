// src/managers/storage/StorageManager.cpp
// MODIFIED FILE
#include "managers/storage/StorageManager.h"
#include "DebugMacros.h"
#include <ArduinoJson.h>
#include "config/hardware_config.h"
#include "helpers/Base64.h"
#include "diagnostics/StorageDiagnostics.h"
#include "helpers/FileNamer.h"
#include "helpers/AtomicSave.h"

// --- Filename for the shutdown flag ---
const char* SHUTDOWN_FLAG_FILENAME = "/shutdown.ok";

// ... (Constructor, Destructor, begin(), checkAndClearShutdownFlag() are unchanged) ...
QueueHandle_t StorageManager::_fileQueue = nullptr;
TaskHandle_t StorageManager::_storageTaskHandle = nullptr;

StorageManager::StorageManager(uint8_t cs_pin, SPIClass* spi, SemaphoreHandle_t spi_bus_mutex, SemaphoreHandle_t diag_result_mutex) :
    _cs_pin(cs_pin),
    _spi(spi),
    _spi_bus_mutex(spi_bus_mutex),
    _diag_result_mutex(diag_result_mutex),
    _diagnostics(nullptr)
{
    _last_diag_result.timestamp = 0;
    _last_diag_result.is_passed = false;
}

StorageManager::~StorageManager() {
    if (_diagnostics) {
        delete _diagnostics;
    }
}

bool StorageManager::begin() {
    LOG_MANAGER("Initializing StorageManager...\n");
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    delay(10);

    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);

    SdSpiConfig spiConfig(_cs_pin, DEDICATED_SPI, 25000000UL, _spi);
    if (!_sd.begin(spiConfig)) {
        LOG_MAIN("[SM_ERROR] SD Card initialization failed.\n");
        _is_ready = false;
        xSemaphoreGive(_spi_bus_mutex);
        return false;
    }
    xSemaphoreGive(_spi_bus_mutex);

    _is_ready = true;
    LOG_MANAGER("SD Card initialized.\n");

    _diagnostics = new StorageDiagnostics(&_sd, _is_ready);

    _fileQueue = xQueueCreate(10, sizeof(FileOperationRequest));
    if (_fileQueue == nullptr) {
        LOG_MAIN("[SM_ERROR] Failed to create file queue.\n");
        return false;
    }

    xTaskCreatePinnedToCore(
        storageTask,
        "StorageTask",
        4096,
        this,
        5,
        &_storageTaskHandle,
        0
    );

    return true;
}
bool StorageManager::checkAndClearShutdownFlag() {
    if (!_is_ready) return false; // If SD isn't ready, it was not a clean shutdown.

    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    bool flag_existed = _sd.exists(SHUTDOWN_FLAG_FILENAME);

    if (flag_existed) {
        _sd.remove(SHUTDOWN_FLAG_FILENAME);
    }
    xSemaphoreGive(_spi_bus_mutex);

    if (flag_existed) {
        LOG_MANAGER("Clean shutdown flag found. Previous session ended correctly.\n");
    } else {
        LOG_MAIN("[SM_WARNING] Shutdown flag not found. Previous session may have crashed.\n");
    }

    return flag_existed;
}


bool StorageManager::writeShutdownFlag() {
    if (!_is_ready) {
        LOG_MAIN("[SM_ERROR] Cannot write shutdown flag, SD not ready.\n");
        return false;
    }

    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);

    FsFile file = _sd.open(SHUTDOWN_FLAG_FILENAME, O_WRITE | O_CREAT);
    bool success = false;
    if (file) {
        // <<< MODIFIED: Add an explicit sync call >>>
        // This forces the SD card controller to flush its cache to the
        // physical flash memory, ensuring the write is permanent before we proceed.
        if (file.sync()) {
            LOG_MANAGER("Shutdown flag synced to SD card.\n");
            success = true;
        } else {
            LOG_MAIN("[SM_ERROR] Failed to sync shutdown flag file.\n");
            success = false;
        }
        file.close();
    } else {
        LOG_MAIN("[SM_ERROR] Failed to create shutdown flag file.\n");
    }

    xSemaphoreGive(_spi_bus_mutex);
    return success;
}

// ... (rest of the file is unchanged) ...
void StorageManager::storageTask(void* pvParameters) {
    StorageManager* self = static_cast<StorageManager*>(pvParameters);
    FileOperationRequest request;

    for (;;) {
        if (xQueueReceive(self->_fileQueue, &request, portMAX_DELAY) == pdPASS) {
            xSemaphoreTake(self->_spi_bus_mutex, portMAX_DELAY);
            
            switch(request.op_type) {
                case FileOperationType::WRITE:
                    AtomicSave::performSave(&self->_sd, request.config_type, request.data.data(), request.data.size());
                    break;
                case FileOperationType::DIAGNOSTICS:
                    self->runAndStoreDiagnostics();
                    break;
            }
            xSemaphoreGive(self->_spi_bus_mutex);
        }
    }
}

void StorageManager::runAndStoreDiagnostics() {
    if (_diagnostics) {
        StorageDiagnosticResult temp_result = _diagnostics->performDiagnostics();
        
        xSemaphoreTake(_diag_result_mutex, portMAX_DELAY);
        _last_diag_result.is_sd_card_initialized = temp_result.is_sd_card_initialized;
        _last_diag_result.can_create_file = temp_result.can_create_file;
        _last_diag_result.can_write_file = temp_result.can_write_file;
        _last_diag_result.can_read_file = temp_result.can_read_file;
        _last_diag_result.can_delete_file = temp_result.can_delete_file;
        _last_diag_result.is_passed = temp_result.is_passed;
        _last_diag_result.free_space_kb = temp_result.free_space_kb;
        _last_diag_result.timestamp = temp_result.timestamp;
        xSemaphoreGive(_diag_result_mutex);

    } else {
        LOG_MAIN("[SM_ERROR] Diagnostics object is null, cannot run test.\n");
    }
}

bool StorageManager::saveState(ConfigType type, const uint8_t* data, size_t len) {
    if (!_is_ready) {
        LOG_MAIN("[SM_ERROR] saveState - Not ready.\n");
        return false;
    }

    FileOperationRequest request;
    request.op_type = FileOperationType::WRITE;
    request.config_type = type;
    request.data.assign(data, data + len);

    if (xQueueSend(_fileQueue, &request, (TickType_t)10) != pdPASS) {
        LOG_MAIN("[SM_ERROR] Failed to send to file queue.\n");
        return false;
    }

    return true;
}

bool StorageManager::requestDiagnostics() {
    FileOperationRequest request;
    request.op_type = FileOperationType::DIAGNOSTICS;

    if (xQueueSend(_fileQueue, &request, (TickType_t)10) != pdPASS) {
        LOG_MAIN("[SM_ERROR] Failed to send diagnostic request to file queue.\n");
        return false;
    }
    return true;
}

StorageDiagnosticResult StorageManager::getDiagnosticResult() const {
    StorageDiagnosticResult result;
    
    xSemaphoreTake(_diag_result_mutex, portMAX_DELAY);
    result.is_sd_card_initialized = _last_diag_result.is_sd_card_initialized;
    result.can_create_file = _last_diag_result.can_create_file;
    result.can_write_file = _last_diag_result.can_write_file;
    result.can_read_file = _last_diag_result.can_read_file;
    result.can_delete_file = _last_diag_result.can_delete_file;
    result.is_passed = _last_diag_result.is_passed;
    result.free_space_kb = _last_diag_result.free_space_kb;
    result.timestamp = _last_diag_result.timestamp;
    xSemaphoreGive(_diag_result_mutex);
    
    return result;
}

bool StorageManager::loadState(ConfigType type, uint8_t* data, size_t len) {
    if (!_is_ready) {
        LOG_MAIN("[SM_ERROR] loadState - Not ready.\n");
        return false;
    }

    const char* fileName = FileNamer::getFileName(type);
    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    if (!_sd.exists(fileName)) {
        LOG_MANAGER("loadState - File %s not found, using defaults.\n", fileName);
        xSemaphoreGive(_spi_bus_mutex);
        return false;
    }

    FsFile file = _sd.open(fileName, O_READ);
    if (!file) {
        LOG_MAIN("[SM_ERROR] loadState - Failed to open %s\n", fileName);
        xSemaphoreGive(_spi_bus_mutex);
        return false;
    }

    size_t bytesRead = file.read(data, len);
    file.close();
    xSemaphoreGive(_spi_bus_mutex);

    if (bytesRead != len) {
        LOG_MAIN("[SM_ERROR] loadState - Read mismatch in %s\n", fileName);
        return false;
    }

    LOG_MANAGER("State loaded from %s\n", fileName);
    return true;
}

bool StorageManager::restoreBackup(const std::vector<uint8_t>& backupData) {
    if (!_is_ready) {
        LOG_MAIN("restoreBackup - Not ready.\n");
        return false;
    }

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, backupData.data(), backupData.size());

    if (error) {
        LOG_MAIN("restoreBackup - Failed to parse backup JSON: %s\n", error.c_str());
        return false;
    }

    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kvp : root) {
        const char* fileName = kvp.key().c_str();
        std::string base64Data = kvp.value().as<std::string>();

        ConfigType type;
        bool found = false;
        for (int i = static_cast<int>(ConfigType::POWER_MANAGER_STATE); i <= static_cast<int>(ConfigType::FILTER_TUNING_CONFIG); ++i) {
            ConfigType currentType = static_cast<ConfigType>(i);
            if (strcmp(FileNamer::getFileName(currentType), fileName) == 0) {
                type = currentType;
                found = true;
                break;
            }
        }

        if (found) {
            LOG_MANAGER("Restoring file: %s\n", fileName);
            std::vector<uint8_t> decodedData = base64_decode(base64Data);

            if (!decodedData.empty() && !saveState(type, decodedData.data(), decodedData.size())) {
                LOG_MAIN("[SM_ERROR] Failed to restore file %s\n", fileName);
            } else if (decodedData.empty()) {
                LOG_MAIN("[SM_WARNING] Skipped restoring empty or invalid decoded data for file %s\n", fileName);
            }
        } else {
            LOG_MANAGER("Skipping unknown file in backup: %s\n", fileName);
        }
    }

    return true;
}


void StorageManager::recoverFromCrash() {
    if (!_is_ready) {
        return;
    }
    LOG_MANAGER("Checking for incomplete writes from previous crash...\n");

    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    for (int i = 0; i <= static_cast<int>(ConfigType::FILTER_TUNING_CONFIG); ++i) {
        ConfigType type = static_cast<ConfigType>(i);
        const char* tempFileName = FileNamer::getTempFileName(type);
        const char* finalFileName = FileNamer::getFileName(type);

        if (_sd.exists(tempFileName)) {
            LOG_MANAGER("Found temporary file %s. Restoring...\n", tempFileName);
            if (_sd.exists(finalFileName)) {
                _sd.remove(finalFileName);
            }
            _sd.rename(tempFileName, finalFileName);
        }
    }
    xSemaphoreGive(_spi_bus_mutex);
}

std::vector<uint8_t> StorageManager::readFile(ConfigType type) {
    std::vector<uint8_t> buffer;
    if (!_is_ready) {
        LOG_MAIN("readFile - Not ready.\n");
        return buffer;
    }

    const char* fileName = FileNamer::getFileName(type);
    xSemaphoreTake(_spi_bus_mutex, portMAX_DELAY);
    if (!_sd.exists(fileName)) {
        LOG_MANAGER("readFile - File %s not found.\n", fileName);
        xSemaphoreGive(_spi_bus_mutex);
        return buffer;
    }

    FsFile file = _sd.open(fileName, O_READ);
    if (!file) {
        LOG_MAIN("readFile - Failed to open %s\n", fileName);
        xSemaphoreGive(_spi_bus_mutex);
        return buffer;
    }

    buffer.resize(file.size());
    file.read(buffer.data(), buffer.size());
    file.close();
    xSemaphoreGive(_spi_bus_mutex);

    LOG_MANAGER("Read %d bytes from %s\n", buffer.size(), fileName);
    return buffer;
}