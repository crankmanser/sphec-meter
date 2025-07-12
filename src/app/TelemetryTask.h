#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Telemetry RTOS task.
 *
 * This task is responsible for periodically generating the telemetry JSON string
 * by calling the TelemetrySerializer and then publishing it to all active
*connectivity managers (Wi-Fi, MQTT, etc.).
 *
 * @param pvParameters A pointer to task parameters (not used).
 */
void telemetryTask(void* pvParameters);