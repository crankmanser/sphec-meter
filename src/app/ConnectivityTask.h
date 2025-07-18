// src/app/ConnectivityTask.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Connectivity RTOS task.
 * @param pvParameters A void pointer to the AppContext struct.
 */
void connectivityTask(void* pvParameters);