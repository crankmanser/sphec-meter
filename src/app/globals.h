// src/app/globals.h
// MODIFIED FILE
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <SPI.h>
#include <Wire.h>

#include "app/common/SystemState.h"
#include "config/Network_Config.h"
#include "data_models/SensorData_types.h"

// --- Extern declarations for all global variables ---
// <<< FIX: The type is corrected to match the definition in main.cpp >>>
extern SPIClass spi;
extern TwoWire i2c;
extern SemaphoreHandle_t g_spi_bus_mutex;
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;
extern SemaphoreHandle_t g_storage_diag_mutex;

extern BootMode g_boot_mode;
extern NetworkConfig networkConfig;
extern RawSensorData g_raw_sensor_data;
extern ProcessedSensorData g_processed_data;