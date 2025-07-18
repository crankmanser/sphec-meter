// src/app/modes/normal.cpp
// MODIFIED FILE
#include "normal.h"
#include "app/AppContext.h" // <<< ADDED
#include "boot/init_hals.h"
#include "boot/init_tasks.h" // <<< ADDED
#include "DebugMacros.h"

void run_normal_mode(AppContext* appContext) {
    LOG_MAIN("--- Entering NORMAL Application Mode ---\n");

    // Initialize HALs and create the application tasks, passing the context down.
    init_hals(appContext);
    init_tasks(appContext); // Tasks are now created within the normal mode flow.

    LOG_MAIN("NORMAL mode initialization complete. Handing control to FreeRTOS.\n");
}