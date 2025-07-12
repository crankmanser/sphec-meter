// src/managers/storage/helpers/FileNamer.h
#pragma once

#include "managers/storage/StorageManager_types.h"

namespace FileNamer {
    const char* getFileName(ConfigType type);
    const char* getTempFileName(ConfigType type);
}