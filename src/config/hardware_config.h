#pragma once

// =================================================================
// SpHEC Meter v1.0.0 - Hardware & Pin Definitions
// =================================================================

// --- I2C Bus ---
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define INA219_I2C_ADDRESS 0x40

// --- I2C Multiplexer (TCA9548A) ---
#define TCA_ADDRESS 0x70
#define RTC_TCA_CHANNEL 0
// Note: OLEDs are not yet assigned channels in the new architecture.
// We will define these when we build the DisplayManager.

// --- SPI Bus (VSPI) ---
#define SPI_MOSI_PIN 23
#define SPI_MISO_PIN 19
#define SPI_SCK_PIN 18
#define SD_CS_PIN 5
#define ADC1_CS_PIN 4  // For pH and 3.3V Bus
#define ADC2_CS_PIN 2  // For EC and 5V Bus

// --- 1-Wire & Proprietary Buses ---
#define ONEWIRE_BUS_PIN 15
#define DHT_PIN 13
#define DHT_TYPE DHT11

// --- User Input ---
#define ENCODER_A_PIN 25
#define ENCODER_B_PIN 26
#define BTN_TOP_PIN 36
#define BTN_MIDDLE_PIN 39
#define BTN_BOTTOM_PIN 34

// --- LEDs ---
#define LED_TOP_RED_PIN 12
#define LED_TOP_GREEN_PIN 14
#define LED_BOTTOM_GREEN_PIN 27