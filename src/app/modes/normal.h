// src/app/modes/normal.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Initializes and runs the main application mode.
 *
 * This function is the primary entry point for the device's normal
 * operation. It initializes all required managers, creates all RTOS tasks,
 * and then yields control to the FreeRTOS scheduler.
 * @param appContext A pointer to the central application context.
 */
void run_normal_mode(AppContext* appContext);