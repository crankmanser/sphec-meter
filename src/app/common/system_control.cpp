// src/app/common/system_control.cpp
// NEW FILE
#include "system_control.h"
#include "managers/storage/StorageManager.h"
#include "presentation/blocks/ShutdownBlock.h"
#include "presentation/DisplayManager.h"
#include "DebugMacros.h"

// External declarations for the managers this handler will use.
extern StorageManager* storageManager;
extern DisplayManager* displayManager;

void initiate_shutdown() {
    LOG_MAIN("--- Clean Shutdown Sequence Initiated ---\n");

    // --- STAGE 1: WRITE SHUTDOWN FLAG ---
    // This is the most critical step. We write a file to the SD card
    // to indicate that the device was powered down cleanly.
    if (storageManager) {
        storageManager->writeShutdownFlag();
    }

    // --- STAGE 2: DISPLAY SHUTDOWN UI ---
    // Use the dedicated ShutdownBlock to inform the user.
    if (displayManager) {
        ShutdownBlockProps props;
        props.message = "Shutting Down";
        ShutdownBlock::draw(displayManager, props);
    }

    // --- STAGE 3: HALT ---
    // Wait a moment for the user to see the message, then enter an
    // infinite loop to halt the system.
    delay(2000);
    LOG_MAIN("System halted.\n");
    while(true) {
        // To be extra safe, we can put the device into a low-power state here
        // or simply let the loop idle.
        delay(1000);
    }
}