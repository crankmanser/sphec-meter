// File Path: /lib/DisplayManager/DisplayManager.h

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "ProjectConfig.h"
#include <FaultHandler.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

/**
 * @class DisplayManager
 * @brief Manages all three OLED displays via the TCA9548A I2C multiplexer.
 * * This cabinet is responsible for initializing the displays and providing
 * high-level functions to draw content on them. It handles the low-level
 * channel switching of the I2C multiplexer.
 */
class DisplayManager {
public:
    /**
     * @brief Constructor for the DisplayManager.
     */
    DisplayManager();

    /**
     * @brief Initializes the I2C bus, multiplexer, and all three displays.
     * * @param faultHandler A reference to the global fault handler.
     * @return True if all displays are initialized successfully, false otherwise.
     */
    bool begin(FaultHandler& faultHandler);

    /**
     * @brief Shows a boot screen message on all displays.
     */
    void showBootScreen();

    /**
     * @brief Gets a pointer to a specific display object.
     * * @param displayIndex The index of the display (0, 1, or 2).
     * @return A pointer to the Adafruit_SSD1306 object, or nullptr if index is invalid.
     */
    Adafruit_SSD1306* getDisplay(uint8_t displayIndex);

private:
    /**
     * @brief Selects a specific channel on the I2C multiplexer.
     * * @param channel The channel number to select (0-7).
     */
    void selectTCAChannel(uint8_t channel);

    FaultHandler* _faultHandler;
    bool _initialized;

    // We have three displays, so we create three Adafruit_SSD1306 objects.
    // The -1 for OLED_RESET_PIN indicates that the displays share a reset pin.
    Adafruit_SSD1306 _display1; // Top OLED
    Adafruit_SSD1306 _display2; // Middle OLED
    Adafruit_SSD1306 _display3; // Bottom OLED
};

#endif // DISPLAY_MANAGER_H