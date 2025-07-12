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

// Forward declarations of global manager pointers and mutexes from main.cpp
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
        // --- STAGE 1: RAW SENSOR READING ---
        // This stage is responsible for acquiring data from all physical sensors.
        // It's protected by a mutex to ensure thread-safe access to the raw data struct.
        if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
            rawSensorReader->update();
            xSemaphoreGive(g_raw_data_mutex);
        }

        // --- STAGE 2: INDEPENDENT PROCESSING ---
        // This stage processes the raw data into final scientific values.
        // It requires locking both the raw and processed data structs to ensure
        // it's working with a consistent set of data.
        if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(g_processed_data_mutex, portMAX_DELAY) == pdTRUE) {
                liquidTempManager->update();
                ambientTempManager->update();
                ambientHumidityManager->update();
                sensorProcessor->update();
                ldrManager->update();
                xSemaphoreGive(g_processed_data_mutex);
            }
            xSemaphoreGive(g_raw_data_mutex);
        }

        
        // Update the power manager as part of the core data production cycle.
        powerManager->update();
        
        // Wait for the next sensor read cycle
        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
    }
}