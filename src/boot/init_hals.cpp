// src/boot/init_hals.cpp
// MODIFIED FILE
#include "init_hals.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

// External declarations for HAL driver pointers
extern ADS1118_Driver* adc1;
extern ADS1118_Driver* adc2;
extern DS18B20_Driver* ds18b20;
extern DHT_Driver* dht;
// ldr does not have a begin() method

void init_hals() {
    // <<< FIX: Instantiations have been moved to main.cpp. >>>
    // This function now only calls the begin() methods.

    if (adc1) adc1->begin();
    if (adc2) adc2->begin();
    if (ds18b20) ds18b20->begin();
    if (dht) dht->begin();

    LOG_MAIN("Non-I2C HAL drivers initialized.\n");
}