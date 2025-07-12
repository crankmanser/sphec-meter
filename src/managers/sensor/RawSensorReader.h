#pragma once

#include "data_models/SensorData_types.h"
#include "hal/ADS1118_Driver.h"
#include "hal/INA219_Driver.h"
#include "hal/LDR_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"

// This manager is responsible for all raw sensor data acquisition.
// It is now completely thread-agnostic.
class RawSensorReader {
public:

    RawSensorReader(
        RawSensorData* target_data_struct,
        ADS1118_Driver* adc1,
        ADS1118_Driver* adc2,
        INA219_Driver* ina219,
        LDR_Driver* ldr,
        DS18B20_Driver* ds18b20,
        DHT_Driver* dht
    );

    void begin();

    // This single method reads from all sensors and updates the data struct.
    void update();

private:
    RawSensorData* _data;

    ADS1118_Driver* _adc1;
    ADS1118_Driver* _adc2;
    INA219_Driver* _ina219;
    LDR_Driver* _ldr;
    DS18B20_Driver* _ds18b20;
    DHT_Driver* _dht;
};