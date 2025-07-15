// src/app/modes/normal.cpp
// MODIFIED FILE
#include "normal.h"
#include "boot/init_hals.h"
#include "boot/init_tasks.h"
#include "boot/init_managers.h"
#include "managers/io/ButtonManager.h"
#include "managers/io/EncoderManager.h"
#include "DebugMacros.h"

// External declarations for objects created in main.cpp
extern ButtonManager* buttonManager;
extern EncoderManager* encoderManager;

// Foward declare to match the required signature
class NoiseAnalysisManager;

void run_normal_mode() {
    LOG_MAIN("--- Entering NORMAL Application Mode ---\n");

    // --- STAGE 1: INITIALIZE HARDWARE & PRE-REQUISITES ---
    init_hals();
    buttonManager->begin();
    encoderManager->begin();

    // --- STAGE 2: CREATE TASKS & INITIALIZE MANAGERS ---
    // It's crucial that init_tasks() runs before init_managers().
    // This ensures that all background tasks (like StorageTask and UiTask)
    // are created and waiting for commands before any manager tries to
    // use them or send them a message on a queue.
    init_tasks();
    init_managers(nullptr); // Pass nullptr as NoiseAnalysisManager is not used in NORMAL mode

    LOG_MAIN("NORMAL mode initialization complete. Handing control to FreeRTOS.\n");
}