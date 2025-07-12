#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Sensor RTOS task.
 *
 * This task orchestrates the entire sensor data pipeline. It periodically
 * triggers the RawSensorReader to acquire fresh data from all hardware sensors
 * and then triggers the various processing managers (SensorProcessor,
 * Temp managers, etc.) to convert that raw data into final, calibrated
 * scientific values.
 *
 * @param pvParameters A pointer to task parameters (not used).
 */
void sensorTask(void* pvParameters);