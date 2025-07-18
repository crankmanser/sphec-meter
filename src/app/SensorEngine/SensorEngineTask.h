// src/app/SensorEngine/SensorEngineTask.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h"

// Forward-declare the classes used in the parameters struct to avoid
// including their full headers here, keeping this header lightweight.
class DataProcessor;
class DataPublisher;

/**
 * @brief Defines the parameters passed to the SensorEngineTask.
 *
 * This struct bundles all the necessary dependencies (helper modules) that
 * the task needs to operate.
 */
struct SensorEngineParams {
    DataProcessor* processor;
    DataPublisher* publisher;
};


/**
 * @brief The main function for the new Sensor Engine RTOS task.
 *
 * This task orchestrates the entire sensor data pipeline in an atomic,
 * thread-safe manner. It is responsible for triggering data acquisition,
 * processing, and publication by calling dedicated helper modules.
 * This design resolves the race condition present in the previous
 * multi-task implementation.
 *
 * @param pvParameters A pointer to a SensorEngineParams struct.
 */
void sensorEngineTask(void* pvParameters);