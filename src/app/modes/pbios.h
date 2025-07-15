// src/app/modes/pbios.h
// NEW FILE
#pragma once

/**
 * @brief Initializes and runs the pBios (diagnostics) mode.
 *
 * This function contains a simple, blocking loop for the pBios UI.
 * It initializes only the minimal set of managers required for diagnostics
 * and does NOT create the main application's RTOS tasks.
 */
void run_pbios_mode();