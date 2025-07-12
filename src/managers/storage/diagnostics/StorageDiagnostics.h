// src/managers/storage/diagnostics/StorageDiagnostics.h
#pragma once

#include "managers/storage/StorageManager_types.h" // <<< MODIFIED: Include the types header
#include <SdFat.h>

class StorageDiagnostics {
public:
    StorageDiagnostics(SdFs* sd, bool is_ready);

    // Runs the full suite of diagnostic tests.
    // <<< MODIFIED: The return type is now correctly defined before use.
    StorageDiagnosticResult performDiagnostics();

private:
    SdFs* _sd;
    bool _is_ready;
};