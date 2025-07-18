// src/boot/init_hals.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Instantiates and initializes all HAL drivers.
 * @param appContext A pointer to the central application context.
 */
void init_hals(AppContext* appContext);