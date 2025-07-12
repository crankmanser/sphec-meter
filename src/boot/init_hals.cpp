// src/boot/init_hals.cpp
// MODIFIED FILE
#include "init_hals.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

// External declarations for HAL driver pointers and hardware handles
extern ADS1118_Driver* adc1;
extern ADS1118_Driver* adc2;
extern DS18B20_Driver* ds18b20;
extern DHT_Driver* dht;
extern LDR_Driver* ldr;

extern SPIClass& spi;
extern SemaphoreHandle_t g_spi_bus_mutex;

void init_hals() {
    // HAL Instantiation (SPI and 1-Wire devices only)
    adc1 = new ADS1118_Driver(ADC1_CS_PIN, ADC2_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    adc2 = new ADS1118_Driver(ADC2_CS_PIN, ADC1_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    ds18b20 = new DS18B20_Driver(ONEWIRE_BUS_PIN);
    dht = new DHT_Driver(DHT_PIN, DHT_TYPE);
    ldr = new LDR_Driver(adc1);

    // HAL Initialization
    adc1->begin();
    adc2->begin();
    ds18b20->begin();
    dht->begin();

    LOG_MAIN("Non-I2C HAL drivers initialized.\n");
}