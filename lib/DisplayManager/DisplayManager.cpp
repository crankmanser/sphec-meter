// File Path: /lib/DisplayManager/DisplayManager.cpp

#include "DisplayManager.h"

//
// --- CONSTRUCTOR ---
//
DisplayManager::DisplayManager() : 
    _faultHandler(nullptr), 
    _initialized(false),
    // Initialize display objects with their dimensions and the Wire library.
    _display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _display3(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
{}

//
// --- I2C MULTIPLEXER CHANNEL SELECTION ---
//
void DisplayManager::selectTCAChannel(uint8_t channel) {
    if (channel > 7) return;
    Wire.beginTransmission(TCA_ADDRESS);
    Wire.write(1 << channel);
    Wire.endTransmission();
}

//
// --- INITIALIZATION ---
//
bool DisplayManager::begin(FaultHandler& faultHandler) {
    _faultHandler = &faultHandler;

    // Initialize the I2C bus itself
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    bool success = true;

    // Initialize Display 1 (Top)
    selectTCAChannel(OLED3_TCA_CHANNEL);
    if (!_display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // In a real scenario, you might use the fault handler here.
        // For now, we'll just track success.
        success = false;
    }

    // Initialize Display 2 (Middle)
    selectTCAChannel(OLED2_TCA_CHANNEL);
    if (!_display2.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        success = false;
    }

    // Initialize Display 3 (Bottom)
    selectTCAChannel(OLED1_TCA_CHANNEL);
    if (!_display3.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        success = false;
    }

    _initialized = success;
    return success;
}

//
// --- PUBLIC METHODS ---
//
void DisplayManager::showBootScreen() {
    if (!_initialized) return;

    Adafruit_SSD1306* displays[] = {&_display1, &_display2, &_display3};
    uint8_t channels[] = {OLED3_TCA_CHANNEL, OLED2_TCA_CHANNEL, OLED1_TCA_CHANNEL};

    for (int i = 0; i < 3; ++i) {
        selectTCAChannel(channels[i]);
        displays[i]->clearDisplay();
        displays[i]->setTextSize(1);
        displays[i]->setTextColor(SSD1306_WHITE);
        displays[i]->setCursor(0, 10);
        displays[i]->println("SpHEC Meter v2.1.0");
        displays[i]->println("Booting...");
        displays[i]->display();
    }
}

Adafruit_SSD1306* DisplayManager::getDisplay(uint8_t displayIndex) {
    switch (displayIndex) {
        case 0: return &_display1; // Top
        case 1: return &_display2; // Middle
        case 2: return &_display3; // Bottom
        default: return nullptr;
    }
}