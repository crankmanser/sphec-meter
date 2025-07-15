// src/boot/init_globals.cpp
// MODIFIED FILE
#include "init_globals.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <SPI.h>
#include "config/hardware_config.h"
#include "DebugMacros.h"

// External declarations for globals defined in main.cpp
extern SPIClass& spi;
extern SemaphoreHandle_t g_spi_bus_mutex;
extern SemaphoreHandle_t g_raw_data_mutex;
extern SemaphoreHandle_t g_processed_data_mutex;
extern SemaphoreHandle_t g_storage_diag_mutex;

void init_globals() {
    // --- STAGE 1: CREATE RTOS PRIMITIVES ---
    // These mutexes are essential for thread-safe access to shared resources.
    g_spi_bus_mutex = xSemaphoreCreateMutex();
    g_raw_data_mutex = xSemaphoreCreateMutex();
    g_processed_data_mutex = xSemaphoreCreateMutex();
    g_storage_diag_mutex = xSemaphoreCreateMutex();

    // --- STAGE 2: INITIALIZE HARDWARE PINS ---
    // Set up all chip select pins for the SPI bus to a known high state.
    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(ADC1_CS_PIN, OUTPUT);
    pinMode(ADC2_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    digitalWrite(ADC1_CS_PIN, HIGH);
    digitalWrite(ADC2_CS_PIN, HIGH);

    // <<< ADDED: Initialize button pins as early as possible >>>
    // This ensures a reliable reading for boot mode detection.
    pinMode(BTN_MIDDLE_PIN, INPUT_PULLUP);
    pinMode(BTN_BOTTOM_PIN, INPUT_PULLUP);
    delay(10); // Small delay for pullups to stabilize

    // --- STAGE 3: INITIALIZE SPI BUS ---
    // Initialize the physical SPI bus, but not any specific devices yet.
    spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);
    LOG_MAIN("Global primitives and SPI bus initialized.\n");
}