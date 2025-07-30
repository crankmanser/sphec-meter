// File Path: /lib/DisplayManager/DisplayManager.h
// MODIFIED FILE

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "ProjectConfig.h"
#include <FaultHandler.h>
// Removed FreeRTOS includes as mutex is no longer handled here

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

class DisplayManager {
public:
    DisplayManager();
    // --- FIX: The begin method no longer needs the mutex ---
    bool begin(FaultHandler& faultHandler);
    void showBootScreen();
    Adafruit_SSD1306* getDisplay(uint8_t displayIndex);
    // --- FIX: This is now a simple, non-locking channel switch ---
    void selectTCAChannel(uint8_t channel);

private:
    FaultHandler* _faultHandler;
    bool _initialized;
    Adafruit_SSD1306 _display1; // Top OLED
    Adafruit_SSD1306 _display2; // Middle OLED
    Adafruit_SSD1306 _display3; // Bottom OLED
};

#endif // DISPLAY_MANAGER_H