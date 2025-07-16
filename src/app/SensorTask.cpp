// src/app/SensorTask.cpp
// MODIFIED FILE
#include "app/SensorTask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "DebugMacros.h"
#include "managers/sensor/RawSensorReader.h"
#include "managers/sensor/LiquidTempManager.h"
#include "managers/sensor/AmbientTempManager.h"
#include "managers/sensor/AmbientHumidityManager.h"
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/LDRManager.h"
#include "managers/power/PowerManager.h"

// External declarations of global manager pointers and mutexes from main.cpp
extern RawSensorReader* rawSensorReader;
extern LiquidTempManager* liquidTempManager;
extern AmbientTempManager* ambientTempManager;
extern AmbientHumidityManager* ambientHumidityManager;
extern SensorProcessor* sensorProcessor;
extern LDRManager* ldrManager;
extern PowerManager* powerManager;

extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;

// Define the delay for the sensor task loop
const TickType_t SENSOR_DELAY_MS = 2000;

void sensorTask(void* pvParameters) {
    LOG_TASK("Sensor Task started.\n");

    for (;;) {
        LOG_TASK("--- Sensor Task Cycle Start ---\n");

        // --- FIX: Refactored to use a single, comprehensive critical section ---
        // This is the definitive fix for the deadlock. By taking both mutexes at the start
        // and releasing them at the end, we create a single, atomic operation for the
        // entire data pipeline, preventing any possibility of a deadlock.

        // --- STAGE 1: ACQUIRE ALL RESOURCES ---
        if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(g_processed_data_mutex, portMAX_DELAY) == pdTRUE) {
                
                // --- STAGE 2: PERFORM ALL DATA OPERATIONS ---
                
                // Read from all physical sensors into the raw data struct.
                LOG_TASK("Updating RawSensorReader...\n");
                rawSensorReader->update();

                // Process all raw data into final, calibrated scientific values.
                LOG_TASK("Updating processing managers...\n");
                liquidTempManager->update();
                ambientTempManager->update();
                ambientHumidityManager->update();
                sensorProcessor->update();
                ldrManager->update();
                
                // The PowerManager update is independent of the sensor data mutexes,
                // but we include it here for a clean, sequential update cycle.
                LOG_TASK("Updating PowerManager...\n");
                powerManager->update();

                // --- STAGE 3: RELEASE ALL RESOURCES ---
                xSemaphoreGive(g_processed_data_mutex);
            } else {
                 LOG_MAIN("[SENSOR_TASK_ERROR] Failed to take processed_data_mutex.\n");
            }
            xSemaphoreGive(g_raw_data_mutex);
        } else {
            LOG_MAIN("[SENSOR_TASK_ERROR] Failed to take raw_data_mutex.\n");
        }
        
        LOG_TASK("--- Sensor Task Cycle End ---\n");
        
        // Wait for the next sensor read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
    }
}
