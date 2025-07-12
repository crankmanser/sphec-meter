// src/presentation/DisplayManager.cpp
// MODIFIED FILE
#include "presentation/DisplayManager.h"
#include "DebugMacros.h"

DisplayManager::DisplayManager(TCA9548_Manual_Driver& tca) : // <<< MODIFIED
    _tca(tca),
    _oled_top(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _oled_middle(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _oled_bottom(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
{}

bool DisplayManager::begin(TwoWire* wire) {
    LOG_MANAGER("Initializing DisplayManager...\n");
    bool success = true;

    selectOLED(OLED_ID::OLED_TOP);
    if (!_oled_top.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        LOG_MAIN("[DM_ERROR] Top OLED allocation failed.\n");
        success = false;
    }

    selectOLED(OLED_ID::OLED_MIDDLE);
    if (!_oled_middle.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        LOG_MAIN("[DM_ERROR] Middle OLED allocation failed.\n");
        success = false;
    }

    selectOLED(OLED_ID::OLED_BOTTOM);
    if (!_oled_bottom.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        LOG_MAIN("[DM_ERROR] Bottom OLED allocation failed.\n");
        success = false;
    }

    if (success) {
        LOG_MANAGER("All OLEDs initialized successfully.\n");
        clearAll();
        displayAll();
    }
    
    _tca.deselectAllChannels();
    return success;
}

void DisplayManager::selectOLED(OLED_ID oled) {
    uint8_t channel = 0;
    switch (oled) {
        case OLED_ID::OLED_TOP:    channel = OLED_TOP_TCA_CHANNEL;    break;
        case OLED_ID::OLED_MIDDLE: channel = OLED_MIDDLE_TCA_CHANNEL; break;
        case OLED_ID::OLED_BOTTOM: channel = OLED_BOTTOM_TCA_CHANNEL; break;
    }
    _tca.selectChannel(channel);
}

void DisplayManager::clearOLED(OLED_ID oled) {
    selectOLED(oled);
    getDisplay(oled)->clearDisplay();
}

void DisplayManager::clearAll() {
    clearOLED(OLED_ID::OLED_TOP);
    clearOLED(OLED_ID::OLED_MIDDLE);
    clearOLED(OLED_ID::OLED_BOTTOM);
}

void DisplayManager::displayOLED(OLED_ID oled) {
    selectOLED(oled);
    getDisplay(oled)->display();
}

void DisplayManager::displayAll() {
    displayOLED(OLED_ID::OLED_TOP);
    displayOLED(OLED_ID::OLED_MIDDLE);
    displayOLED(OLED_ID::OLED_BOTTOM);
}

Adafruit_SSD1306* DisplayManager::getDisplay(OLED_ID oled) {
    switch (oled) {
        case OLED_ID::OLED_TOP:    return &_oled_top;
        case OLED_ID::OLED_MIDDLE: return &_oled_middle;
        case OLED_ID::OLED_BOTTOM: return &_oled_bottom;
    }
    return nullptr;
}