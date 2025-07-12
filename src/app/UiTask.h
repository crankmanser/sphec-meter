// src/app/UiTask.h
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the UI RTOS task.
 *
 * This task orchestrates the entire user interface. It runs in a tight
 * loop, checking for user input, getting the latest render state from the
 * active screen, and commanding the UIManager to draw the frame.
 *
 * @param pvParameters A pointer to task parameters (not used).
 */
void uiTask(void* pvParameters);