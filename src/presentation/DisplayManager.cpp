// src/presentation/DisplayManager.cpp
// MODIFIED FILE
#include "presentation/DisplayManager.h"
#include "DebugMacros.h"

// <<< MODIFIED: The constructor now initializes the Adafruit_SSD1306 objects
// with the provided 'wire' pointer, instead of the global 'Wire'.
DisplayManager::DisplayManager(TCA9548_Manual_Driver& tca, TwoWire* wire) :
    _tca(tca),
    _oled_top(SCREEN_WIDTH, SCREEN_HEIGHT, wire, -1),
    _oled_middle(SCREEN_WIDTH, SCREEN_HEIGHT, wire, -1),
    _oled_bottom(SCREEN_WIDTH, SCREEN_HEIGHT, wire, -1)
{}

void DisplayManager::selectChannel_Direct(uint8_t channel) {
    _tca.selectChannel(channel);
}

bool DisplayManager::begin(TwoWire* wire) {
    LOG_MANAGER("Initializing DisplayManager...\n");
    bool success = true;

    // Initialize Top OLED directly on Channel 7
    selectChannel_Direct(OLED_TOP_TCA_CHANNEL);
    if (!_oled_top.begin(SSD1306_SWITCHCAPVCC, 0x3C, false)) {
        LOG_MAIN("[DM_ERROR] Top OLED allocation failed on channel %d.\n", OLED_TOP_TCA_CHANNEL);
        success = false;
    } else {
        LOG_MANAGER("Top OLED Initialized on Channel %d.", OLED_TOP_TCA_CHANNEL);
    }

    // Initialize Middle OLED directly on Channel 5
    selectChannel_Direct(OLED_MIDDLE_TCA_CHANNEL);
    if (!_oled_middle.begin(SSD1306_SWITCHCAPVCC, 0x3C, false)) {
        LOG_MAIN("[DM_ERROR] Middle OLED allocation failed on channel %d.\n", OLED_MIDDLE_TCA_CHANNEL);
        success = false;
    } else {
        LOG_MANAGER("Middle OLED Initialized on Channel %d.", OLED_MIDDLE_TCA_CHANNEL);
    }

    // Initialize Bottom OLED directly on Channel 2
    selectChannel_Direct(OLED_BOTTOM_TCA_CHANNEL);
    if (!_oled_bottom.begin(SSD1306_SWITCHCAPVCC, 0x3C, false)) {
        LOG_MAIN("[DM_ERROR] Bottom OLED allocation failed on channel %d.\n", OLED_BOTTOM_TCA_CHANNEL);
        success = false;
    } else {
        LOG_MANAGER("Bottom OLED Initialized on Channel %d.", OLED_BOTTOM_TCA_CHANNEL);
    }

    if (success) {
        LOG_MANAGER("All OLEDs initialized successfully.");
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
    selectChannel_Direct(channel);
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