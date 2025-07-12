#include "debug/simulation.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // <<< ADDED: For semaphore types and functions

// --- ADDED: Include all required manager headers ---
#include "managers/sensor/LiquidTempManager.h"
#include "managers/sensor/AmbientTempManager.h"
#include "managers/sensor/AmbientHumidityManager.h"
#include "managers/sensor/SensorProcessor.h"
#include "managers/sensor/LDRManager.h"

#include "data_models/SensorData_types.h"
#include "DebugMacros.h"

// --- Externally-defined global variables ---
extern RawSensorData g_raw_sensor_data;
extern ProcessedSensorData g_processed_data;
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;

// Pointers to managers
extern LiquidTempManager* liquidTempManager;
extern AmbientTempManager* ambientTempManager;
extern AmbientHumidityManager* ambientHumidityManager;
extern SensorProcessor* sensorProcessor;
extern LDRManager* ldrManager;

void run_test_and_halt() {
    // --- 1. SIMULATE RAW SENSOR DATA ---
    if (xSemaphoreTake(g_raw_data_mutex, portMAX_DELAY) == pdTRUE) {
        g_raw_sensor_data.adc_ph_raw = 12345;
        g_raw_sensor_data.adc_ec_raw = 5678;
        g_raw_sensor_data.temp_ds18b20_raw = 24.5f;
        g_raw_sensor_data.temp_dht11_raw = 26.8f;
        g_raw_sensor_data.humidity_dht11_raw = 65.2f;
        g_raw_sensor_data.ldr_voltage_raw = 1.8f;
        LOG_MAIN("TEST: Manually populated g_raw_sensor_data.\n");
        xSemaphoreGive(g_raw_data_mutex);
    }

    // --- 2. RUN THE PROCESSING PIPELINE ---
    if (xSemaphoreTake(g_raw_data_mutex, (TickType_t)10) == pdTRUE) {
        if (xSemaphoreTake(g_processed_data_mutex, (TickType_t)10) == pdTRUE) {
            liquidTempManager->update();
            ambientTempManager->update();
            ambientHumidityManager->update();
            sensorProcessor->update();
            ldrManager->update();
            xSemaphoreGive(g_processed_data_mutex);
        }
        xSemaphoreGive(g_raw_data_mutex);
    }
    LOG_MAIN("TEST: All processing managers have run.\n");

    // --- 3. VERIFY THE OUTPUT ---
    if (xSemaphoreTake(g_processed_data_mutex, portMAX_DELAY) == pdTRUE) {
        LOG_MAIN("--- VERIFICATION --- \n");
        LOG_MAIN("Processed pH: %.2f\n", g_processed_data.ph_value);
        LOG_MAIN("Processed EC: %.2f\n", g_processed_data.ec_value);
        LOG_MAIN("Processed Liquid Temp: %.2f C\n", g_processed_data.liquid_temp_c);
        LOG_MAIN("Processed Ambient Temp: %.2f C\n", g_processed_data.ambient_temp_c);
        LOG_MAIN("Processed Ambient Humidity: %.1f %%\n", g_processed_data.ambient_humidity_percent);
        LOG_MAIN("Processed Light Level: %.1f %%\n", g_processed_data.light_level_percent);
        LOG_MAIN("--- END VERIFICATION --- \n");
        xSemaphoreGive(g_processed_data_mutex);
    }

    // Halt execution after the test completes.
    while(true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}