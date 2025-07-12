// src/boot/init_hals.cpp
// MODIFIED FILE
#include "init_hals.h"
#include "hal/INA219_Driver.h"
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "hal/TCA9548_Manual_Driver.h" // <<< MODIFIED
#include "hal/PCF8563_Driver.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

// External declarations for HAL driver pointers and hardware handles
extern INA219_Driver* ina219;
extern ADS1118_Driver* adc1;
extern ADS1118_Driver* adc2;
extern DS18B20_Driver* ds18b20;
extern DHT_Driver* dht;
extern LDR_Driver* ldr;
extern TCA9548_Manual_Driver* tca9548; // <<< MODIFIED
extern PCF8563_Driver* pcf8563_driver;

extern SPIClass& spi;
extern TwoWire i2c;
extern SemaphoreHandle_t g_spi_bus_mutex;

void init_hals() {
    // HAL Instantiation
    ina219 = new INA219_Driver(INA219_I2C_ADDRESS);
    adc1 = new ADS1118_Driver(ADC1_CS_PIN, ADC2_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    adc2 = new ADS1118_Driver(ADC2_CS_PIN, ADC1_CS_PIN, SD_CS_PIN, &spi, g_spi_bus_mutex);
    ds18b20 = new DS18B20_Driver(ONEWIRE_BUS_PIN);
    dht = new DHT_Driver(DHT_PIN, DHT_TYPE);
    ldr = new LDR_Driver(adc1);
    tca9548 = new TCA9548_Manual_Driver(TCA_ADDRESS, &i2c); // <<< MODIFIED
    pcf8563_driver = new PCF8563_Driver();

    // HAL Initialization
    tca9548->begin(); // <<< MODIFIED
    ina219->begin(&i2c);
    adc1->begin();
    adc2->begin();
    ds18b20->begin();
    dht->begin();
    pcf8563_driver->begin(&i2c);

    LOG_MAIN("All HAL drivers initialized.\n");
}