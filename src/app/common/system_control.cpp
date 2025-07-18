// src/app/common/system_control.cpp
// MODIFIED FILE
#include "system_control.h"
#include "app/AppContext.h"
#include "managers/storage/StorageManager.h"
#include "presentation/blocks/ShutdownBlock.h"
#include "presentation/DisplayManager.h"
#include "DebugMacros.h"

void initiate_shutdown(AppContext* appContext) {
    LOG_MAIN("--- Clean Shutdown Sequence Initiated ---\n");

    // --- STAGE 1: WRITE SHUTDOWN FLAG ---
    if (appContext->storageManager) {
        appContext->storageManager->writeShutdownFlag();
    }

    // --- STAGE 2: DISPLAY SHUTDOWN UI ---
    if (appContext->displayManager) {
        ShutdownBlockProps props;
        props.message = "Shutting Down";
        ShutdownBlock::draw(appContext->displayManager, props);
    }

    // --- STAGE 3: HALT ---
    delay(2000);
    LOG_MAIN("System halted.\n");
    while(true) {
        delay(1000);
    }
}