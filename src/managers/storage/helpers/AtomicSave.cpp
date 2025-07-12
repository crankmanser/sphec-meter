// src/managers/storage/helpers/AtomicSave.cpp
#include "AtomicSave.h"
#include "DebugMacros.h"
#include "managers/storage/helpers/FileNamer.h"

namespace AtomicSave {
    bool performSave(SdFs* sd, ConfigType type, const uint8_t* data, size_t len) {
        const char* tempFileName = FileNamer::getTempFileName(type);
        const char* finalFileName = FileNamer::getFileName(type);

        FsFile file = sd->open(tempFileName, O_WRITE | O_CREAT | O_TRUNC);
        if (!file) {
            LOG_MAIN("[SM_ERROR] performSave - Failed to open temp file %s\n", tempFileName);
            return false;
        }
        size_t bytesWritten = file.write(data, len);
        file.close();

        if (bytesWritten != len) {
            LOG_MAIN("[SM_ERROR] performSave - Write failed for %s\n", tempFileName);
            sd->remove(tempFileName);
            return false;
        }

        if (sd->exists(finalFileName)) {
            if (!sd->remove(finalFileName)) {
                LOG_MAIN("[SM_ERROR] performSave - Failed to remove old file %s\n", finalFileName);
                return false;
            }
        }

        if (!sd->rename(tempFileName, finalFileName)) {
            LOG_MAIN("[SM_ERROR] performSave - Failed to rename %s to %s\n", tempFileName, finalFileName);
            return false;
        }

        LOG_MANAGER("State saved to %s\n", finalFileName);
        return true;
    }
}