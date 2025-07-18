// src/boot/post.h
// MODIFIED FILE
#pragma once

// Forward-declare AppContext
struct AppContext;

/**
 * @brief Runs the Power-On Self-Test.
 * @param appContext A pointer to the central application context struct.
 * @return true if all tests pass, false otherwise.
 */
bool run_post(AppContext* appContext);