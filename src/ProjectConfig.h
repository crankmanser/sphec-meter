// File Path: /src/ProjectConfig.h

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

// =================================================================
// Hardware & Pin Definitions
// =================================================================

// --- I2C Bus ---
// Primary bus for sensors and displays.
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define INA219_I2C_ADDRESS 0x40 // For the PowerMonitor cabinet

// --- I2C Multiplexer (TCA9548A) ---
// Used to manage multiple devices on the same I2C address space.
#define TCA_ADDRESS 0x70
#define RTC_TCA_CHANNEL 0       // RTC (PCF8563) on channel 0
#define OLED1_TCA_CHANNEL 7     // Bottom OLED
#define OLED2_TCA_CHANNEL 5     // Middle OLED
#define OLED3_TCA_CHANNEL 2     // Top OLED

// --- SPI Bus (VSPI) ---
// High-speed bus for ADCs and SD Card.
#define VSPI_MOSI_PIN 23
#define VSPI_MISO_PIN 19
#define VSPI_SCK_PIN 18
#define SD_CS_PIN 5     // Chip Select for SD Card module (HW-203)
#define ADC1_CS_PIN 4     // Chip Select for ADS1118 #1 (3.3V bus & pH)
#define ADC2_CS_PIN 2     // Chip Select for ADS1118 #2 (5V bus & EC)

// --- 1-Wire & Proprietary Buses ---
#define ONE_WIRE_BUS_PIN 15 // For DS18B20 temperature sensor(s)
#define DHT_PIN 13          // For DHT11 Ambient Temp/Humidity Sensor
#define DHT_TYPE DHT11

// --- User Input ---
#define ENCODER_PIN_A 25
#define ENCODER_PIN_B 26
#define BTN_OLED1_PIN 34 // Bottom Button
#define BTN_OLED2_PIN 39 // Middle Button
#define BTN_OLED3_PIN 36 // Top Button

// --- LEDs ---
// Note: Pin mapping corrected based on "Bi-Color (Red/Blue)" description
// and pin definition file.
#define LED_POWER_PIN       27 // Single color Green LED (Bottom)
#define LED_STATUS1_PIN     14 // Bi-color LED Pin 1 (Mapped to Green in file, but used as Blue for status)
#define LED_STATUS2_PIN     12 // Bi-color LED Pin 2 (Red)


// =================================================================
// Software Configuration
// =================================================================

// --- FreeRTOS Task Priorities ---
// High number = higher priority
#define TASK_PRIORITY_HIGH   4
#define TASK_PRIORITY_NORMAL 2
#define TASK_PRIORITY_LOW    1

#define SD_TASK_PRIORITY_HIGH   5 // For critical, uninterruptible writes
#define SD_TASK_PRIORITY_NORMAL 2


#endif // PROJECT_CONFIG_H