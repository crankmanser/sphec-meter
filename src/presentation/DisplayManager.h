// src/presentation/DisplayManager.h
// MODIFIED FILE
#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal/TCA9548_Driver.h"
#include "presentation/common/UI_types.h"
#include "config/hardware_config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/**
 * @class DisplayManager
 * @brief A pure Hardware Abstraction Layer (HAL) for the three OLED screens.
 */
class DisplayManager {
public:
    DisplayManager(TCA9548_Driver& tca);
    bool begin(TwoWire* wire);

    void selectOLED(OLED_ID oled);
    void clearOLED(OLED_ID oled);
    void clearAll();
    void displayOLED(OLED_ID oled);
    void displayAll();
    Adafruit_SSD1306* getDisplay(OLED_ID oled);

private:
    TCA9548_Driver& _tca;
    Adafruit_SSD1306 _oled_top;
    Adafruit_SSD1306 _oled_middle;
    Adafruit_SSD1306 _oled_bottom;

    // <<< FIX: Correct I2C multiplexer channels based on user-provided hardware info.
    static constexpr uint8_t OLED_TOP_TCA_CHANNEL    = 7; // Was 1
    static constexpr uint8_t OLED_MIDDLE_TCA_CHANNEL = 5; // Was 2
    static constexpr uint8_t OLED_BOTTOM_TCA_CHANNEL = 2; // Was 3
};