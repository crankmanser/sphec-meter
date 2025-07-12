// src/presentation/DisplayManager.h
// MODIFIED FILE
#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal/TCA9548_Manual_Driver.h"
#include "presentation/common/UI_types.h"
#include "config/hardware_config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

class DisplayManager {
public:
    // <<< MODIFIED: Constructor now accepts the I2C bus object.
    DisplayManager(TCA9548_Manual_Driver& tca, TwoWire* wire);
    bool begin(TwoWire* wire);

    void selectOLED(OLED_ID oled);
    void clearOLED(OLED_ID oled);
    void clearAll();
    void displayOLED(OLED_ID oled);
    void displayAll();
    Adafruit_SSD1306* getDisplay(OLED_ID oled);

private:
    void selectChannel_Direct(uint8_t channel);

    TCA9548_Manual_Driver& _tca;
    Adafruit_SSD1306 _oled_top;
    Adafruit_SSD1306 _oled_middle;
    Adafruit_SSD1306 _oled_bottom;

    static constexpr uint8_t OLED_TOP_TCA_CHANNEL    = 7;
    static constexpr uint8_t OLED_MIDDLE_TCA_CHANNEL = 5;
    static constexpr uint8_t OLED_BOTTOM_TCA_CHANNEL = 2;
};