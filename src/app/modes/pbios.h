// src/app/modes/pbios.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Initializes and runs the pBios (diagnostics) mode.
 * @param appContext A pointer to the central application context.
 */
void run_pbios_mode(AppContext* appContext);