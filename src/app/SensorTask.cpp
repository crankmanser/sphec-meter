// src/app/SensorTask.cpp
// MODIFIED FILE
#include "app/SensorTask.h"
#include "app/AppContext.h"
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

// Extern declarations for global data structs and mutexes
extern RawSensorData g_raw_sensor_data;
extern ProcessedSensorData g_processed_data;
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;

const TickType_t SENSOR_DELAY_MS = 2000;

void sensorTask(void* pvParameters) {
    LOG_TASK("Sensor Task started.\n");
    AppContext* context = static_cast<AppContext*>(pvParameters);

    for (;;) {
        LOG_TASK("--- Sensor Task Cycle Start ---\n");
        if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(g_processed_data_mutex, portMAX_DELAY) == pdTRUE) {
                
                context->rawSensorReader->update();
                context->liquidTempManager->update();
                context->ambientTempManager->update();
                context->ambientHumidityManager->update();
                context->sensorProcessor->update();
                context->ldrManager->update();
                context->powerManager->update();

                xSemaphoreGive(g_processed_data_mutex);
            }
            xSemaphoreGive(g_raw_data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
    }
}