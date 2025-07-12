// src/managers/RawSensorReader.cpp
#include "RawSensorReader.h"
#include "DebugMacros.h"
#include "config/DebugConfig.h" // For ENABLE_SENSOR_SIMULATION

#if (ENABLE_SENSOR_SIMULATION)
#include "debug/PhCalibrationSimulator.h"
#include "debug/EcCalibrationSimulator.h"
#endif


// <<< MODIFIED: Constructor no longer takes or stores mutex handles.
RawSensorReader::RawSensorReader(
    RawSensorData* target_data_struct,
    ADS1118_Driver* adc1,
    ADS1118_Driver* adc2,
    INA219_Driver* ina219,
    LDR_Driver* ldr,
    DS18B20_Driver* ds18b20,
    DHT_Driver* dht) :
        _data(target_data_struct),
        _adc1(adc1),
        _adc2(adc2),
        _ina219(ina219),
        _ldr(ldr),
        _ds18b20(ds18b20),
        _dht(dht)
{}

void RawSensorReader::begin() {
    LOG_MANAGER("RawSensorReader initialized.\n");
}

void RawSensorReader::update() {
    // <<< MODIFIED: All mutex handling is removed.
    // The calling function in main.cpp is now responsible for thread safety.

    #if (ENABLE_SENSOR_SIMULATION)
        // --- SIMULATION PATH ---
        LOG_MANAGER("--- SENSOR SIMULATION ACTIVE: Injecting simulated raw data. ---\n");
        _data->adc_ph_raw = PhCalibrationSimulator::getSimulatedRawADC();
        _data->adc_ec_raw = EcCalibrationSimulator::getSimulatedRawADC();
        _data->bus_3v3_voltage_raw = 3.3f;
        _data->bus_5v_voltage_raw = 5.0f;
        _data->ina219_current_ma_raw = 150.0f;
        _data->ldr_voltage_raw = 1.75f;
        _data->temp_ds18b20_raw = 25.0f; // Critical for temp compensation
        _data->temp_dht11_raw = 26.5f;
        _data->humidity_dht11_raw = 55.0f;
    #else
        // --- NORMAL OPERATION ---
        // --- Read Analog & Power Sensors ---
        _data->adc_ph_raw = _adc1->readDifferential_0_1();
        _data->adc_ec_raw = _adc2->readDifferential_0_1();
        _data->bus_3v3_voltage_raw = _adc1->readSingleEnded_2();
        _data->bus_5v_voltage_raw = _adc2->readSingleEnded_2();
        _data->ina219_current_ma_raw = _ina219->getCurrent_mA();
        _data->ldr_voltage_raw = _ldr->getVoltage();
        
        // --- Read Temperature & Humidity Sensors ---
        _ds18b20->requestTemperatures();
        _data->temp_ds18b20_raw = _ds18b20->getTempC();
        _data->temp_dht11_raw = _dht->getTemperatureC();
        _data->humidity_dht11_raw = _dht->getHumidity();
    #endif

    // log the raw data collected by RawSensorReader
    LOG_MANAGER("RawSensorReader Data Captured:\n");
    LOG_MANAGER("  PH ADC Raw: %d\n", _data->adc_ph_raw);
    LOG_MANAGER("  EC ADC Raw: %d\n", _data->adc_ec_raw);
    LOG_MANAGER("  DS18B20 Temp Raw: %.2fC\n", _data->temp_ds18b20_raw);

    LOG_MANAGER("Raw sensor data updated.\n");
}