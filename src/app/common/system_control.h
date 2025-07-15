// src/app/common/system_control.h
// NEW FILE
#pragma once

/**
 * @brief Initiates a clean system shutdown sequence.
 *
 * This function orchestrates the entire shutdown process, including
 * writing the shutdown flag, displaying the UI, and halting the system.
 * This is a blocking function and will not return.
 */
void initiate_shutdown();