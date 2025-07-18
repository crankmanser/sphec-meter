// src/app/SensorEngine/SensorEngineTask.cpp
// MODIFIED FILE
#include "SensorEngineTask.h"
#include "DataProcessor.h"
#include "DataPublisher.h"
#include "DebugMacros.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// External declarations for the global mutexes this task will manage.
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;

// <<< REMOVED: The struct definition is now correctly located in the header file.
/*
struct SensorEngineParams {
    DataProcessor* processor;
    DataPublisher* publisher;
};
*/

// This is the master "Sensor Engine Task". It orchestrates the entire
// data pipeline from raw reading to final publication in a single, atomic,
// thread-safe operation.
void sensorEngineTask(void* pvParameters) {
    LOG_TASK("Sensor Engine Task started.\n");

    // Safely cast the void pointer to our parameters struct.
    if (pvParameters == nullptr) {
        LOG_MAIN("[SE_TASK_ERROR] Task parameters are null. Deleting task.\n");
        vTaskDelete(NULL);
        return;
    }
    SensorEngineParams* params = static_cast<SensorEngineParams*>(pvParameters);
    DataProcessor* processor = params->processor;
    DataPublisher* publisher = params->publisher;

    const TickType_t SENSOR_DELAY_MS = 2000;

    for (;;) {
        LOG_TASK("--- Sensor Engine Task Cycle Start ---\n");

        // By taking both data mutexes here, we create a single, atomic operation
        // for the entire data pipeline. This completely resolves the race condition.
        if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(g_processed_data_mutex, portMAX_DELAY) == pdTRUE) {
                
                // --- STAGE 1: Call the DataProcessor ---
                processor->process();

                // --- STAGE 2: Call the DataPublisher ---
                publisher->publish();

                // --- STAGE 3: Release the mutexes in reverse order ---
                xSemaphoreGive(g_processed_data_mutex);
            } else {
                 LOG_MAIN("[SE_TASK_ERROR] Failed to take processed_data_mutex.\n");
            }
            xSemaphoreGive(g_raw_data_mutex);
        } else {
            LOG_MAIN("[SE_TASK_ERROR] Failed to take raw_data_mutex.\n");
        }
        
        LOG_TASK("--- Sensor Engine Task Cycle End ---\n");
        
        // Wait for the next sensor read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
    }
}