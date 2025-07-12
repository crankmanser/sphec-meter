#pragma once

#include <cstdint> // <<< ADDED: Includes standard integer types like int16_t

// This struct holds raw, unprocessed data directly from the HAL.
// It is populated by the RawSensorReader.
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
// It is populated by the specialized managers (PH_Manager, EC_Manager, etc.).
struct ProcessedSensorData {
    float liquid_temp_c;
    float ambient_temp_c;
    float ambient_humidity_percent;
    float ph_value;
    float ec_value;
    float light_level_percent;
    // ... we will add KPI fields here later
};