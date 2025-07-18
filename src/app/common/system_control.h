// src/app/common/system_control.h
// MODIFIED FILE
#pragma once

// Forward-declare the AppContext struct
struct AppContext;

/**
 * @brief Initiates a clean system shutdown sequence.
 *
 * This function orchestrates the entire shutdown process, including
 * writing the shutdown flag, displaying the UI, and halting the system.
 * This is a blocking function and will not return.
 * @param appContext A pointer to the central application context.
 */
void initiate_shutdown(AppContext* appContext);