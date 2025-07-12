// src/managers/storage/helpers/AtomicSave.h
#pragma once

#include <SdFat.h>
#include "managers/storage/StorageManager_types.h"

namespace AtomicSave {
    bool performSave(SdFs* sd, ConfigType type, const uint8_t* data, size_t len);
}