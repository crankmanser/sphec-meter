// src/app/modes/normal.cpp
// MODIFIED FILE
#include "normal.h"
#include "boot/init_hals.h"
#include "boot/init_managers.h"
#include "app/UiTask.h"
#include "DebugMacros.h"

// Foward declare to match the required signature
class NoiseAnalysisManager;

void run_normal_mode() {
    LOG_MAIN("--- Entering NORMAL Application Mode ---\n");

    // --- STAGE 1: INITIALIZE HARDWARE & MANAGERS ---
    init_hals();
    init_managers(nullptr); 

    // --- STAGE 2: CREATE THE UI TASK ---
    // This is created last to ensure all managers and screens are ready.
    // --- FIX: Assign the correct priority from the new scheme ---
    xTaskCreatePinnedToCore(uiTask, "UiTask", 4096, NULL, 3, NULL, 1);

    LOG_MAIN("NORMAL mode initialization complete. Handing control to FreeRTOS.\n");
}
