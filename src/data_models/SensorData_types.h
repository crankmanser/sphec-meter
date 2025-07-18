// src/data_models/SensorData_types.h
// MODIFIED FILE
#pragma once

#include <cstdint> 

// This struct holds raw, unprocessed data directly from the HAL.
struct RawSensorData {
    int16_t adc_ph_raw;
    int16_t adc_ec_raw;
    float temp_ds18b20_raw;
    float temp_dht11_raw;
    float humidity_dht11_raw;
    float ldr_voltage_raw;
    float bus_3v3_voltage_raw;
    float bus_5v_voltage_raw;
    float ina219_current_ma_raw;
};

// This struct holds final, clean, scientific values.
struct ProcessedSensorData {
    float liquid_temp_c;
    float ambient_temp_c;
    float ambient_humidity_percent;
    float ph_value;
    float ec_value;
    float light_level_percent;
};

// <<< ADDED: Extern declarations for global data structs >>>
// These variables are defined in main.cpp. Any file that includes this
// header can now access them. This resolves the linker error.
extern RawSensorData g_raw_sensor_data;
extern ProcessedSensorData g_processed_data;