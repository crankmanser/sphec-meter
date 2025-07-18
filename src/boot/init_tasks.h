// src/boot/init_tasks.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Creates all FreeRTOS application tasks.
 * @param appContext A pointer to the central application context struct.
 */
void init_tasks(AppContext* appContext);