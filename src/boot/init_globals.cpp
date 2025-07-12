// src/boot/init_globals.cpp
#include "init_globals.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <SPI.h>
#include <Wire.h>
#include "config/hardware_config.h"
#include "DebugMacros.h"

// External declarations for globals defined in main.cpp
extern SPIClass& spi;
extern TwoWire i2c;
extern SemaphoreHandle_t g_spi_bus_mutex;
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;
extern SemaphoreHandle_t g_storage_diag_mutex;

void init_globals() {
    g_spi_bus_mutex = xSemaphoreCreateMutex();
    g_raw_data_mutex = xSemaphoreCreateMutex();
    g_processed_data_mutex = xSemaphoreCreateMutex();
    g_storage_diag_mutex = xSemaphoreCreateMutex();

    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(ADC1_CS_PIN, OUTPUT);
    pinMode(ADC2_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    digitalWrite(ADC1_CS_PIN, HIGH);
    digitalWrite(ADC2_CS_PIN, HIGH);
    delay(10);

    spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    i2c.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    LOG_MAIN("Global primitives and buses initialized.\n");
}