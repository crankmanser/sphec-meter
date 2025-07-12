#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Connectivity RTOS task.
 *
 * This task is a housekeeping task responsible for calling the `update()`
 * methods of the connectivity managers. This ensures that Wi-Fi and MQTT
 * connections are maintained.
 *
 * @param pvParameters A pointer to task parameters (not used).
 */
void connectivityTask(void* pvParameters);