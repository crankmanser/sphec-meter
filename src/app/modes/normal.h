// src/app/modes/normal.h
// NEW FILE
#pragma once

/**
 * @brief Initializes and runs the main application mode.
 *
 * This function is the primary entry point for the device's normal
 * operation. It initializes all required managers, creates all RTOS tasks,
 * and then yields control to the FreeRTOS scheduler.
 */
void run_normal_mode();