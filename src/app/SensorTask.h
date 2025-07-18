// src/app/SensorTask.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the Sensor RTOS task.
 * @param pvParameters A void pointer to the AppContext struct.
 */
void sensorTask(void* pvParameters);