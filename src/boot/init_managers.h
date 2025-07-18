// src/boot/init_managers.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Instantiates and initializes all manager cabinets.
 * @param appContext A pointer to the central application context struct.
 */
void init_managers(AppContext* appContext);