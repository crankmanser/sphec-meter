// src/managers/storage/diagnostics/StorageDiagnostics.cpp
#include "StorageDiagnostics.h"
#include "DebugMacros.h"

StorageDiagnostics::StorageDiagnostics(SdFs* sd, bool is_ready) :
    _sd(sd),
    _is_ready(is_ready)
{}

StorageDiagnosticResult StorageDiagnostics::performDiagnostics() {
    StorageDiagnosticResult result;
    result.is_sd_card_initialized = _is_ready;
    result.timestamp = millis(); // Record when the test was run

    if (!_is_ready) {
        LOG_MAIN("[DIAG_ERROR] Test failed: SD card not initialized.\n");
        return result;
    }

    const char* diagFileName = "/diag_test.txt";
    const char* testContent = "SpHEC Meter Storage Test";
    
    LOG_MANAGER("--- Running Storage Diagnostics ---\n");

    FsFile file = _sd->open(diagFileName, O_WRITE | O_CREAT | O_TRUNC);
    if (file) {
        result.can_create_file = true;
        LOG_MANAGER("[DIAG] Create file: PASS\n");
        if (file.write(testContent, strlen(testContent)) == strlen(testContent)) {
            result.can_write_file = true;
            LOG_MANAGER("[DIAG] Write file: PASS\n");
        } else {
            LOG_MAIN("[DIAG_ERROR] Test failed: Could not write to test file.\n");
        }
        file.close();
    } else {
        LOG_MAIN("[DIAG_ERROR] Test failed: Could not create test file.\n");
    }

    if (result.can_write_file) {
        file = _sd->open(diagFileName, O_READ);
        if (file) {
            char buffer[64] = {0};
            file.read(buffer, sizeof(buffer) - 1);
            if (strcmp(buffer, testContent) == 0) {
                result.can_read_file = true;
                LOG_MANAGER("[DIAG] Read file: PASS\n");
            } else {
                 LOG_MAIN("[DIAG_ERROR] Test failed: Read content mismatch.\n");
            }
            file.close();
        } else {
             LOG_MAIN("[DIAG_ERROR] Test failed: Could not re-open file for reading.\n");
        }
    }

    if (_sd->exists(diagFileName)) {
        if (_sd->remove(diagFileName)) {
            result.can_delete_file = true;
            LOG_MANAGER("[DIAG] Delete file: PASS\n");
        } else {
            LOG_MAIN("[DIAG_ERROR] Test failed: Could not delete test file.\n");
        }
    }

    result.free_space_kb = _sd->freeClusterCount() * _sd->bytesPerCluster() / 1024;
    LOG_MANAGER("[DIAG] Free space: %lu KB\n", result.free_space_kb);

    result.is_passed = result.is_sd_card_initialized && result.can_create_file && result.can_write_file && result.can_read_file && result.can_delete_file;
    LOG_MANAGER("--- Storage Diagnostics Complete. Result: %s ---\n", result.is_passed ? "PASS" : "FAIL");

    return result;
}