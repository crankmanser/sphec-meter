// src/app/TelemetryTask.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Telemetry RTOS task.
 * @param pvParameters A void pointer to the AppContext struct.
 */
void telemetryTask(void* pvParameters);