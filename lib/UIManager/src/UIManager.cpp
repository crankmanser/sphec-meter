// File Path: /lib/UIManager/src/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
// --- FIX: Use a relative path to include the header from the src directory ---
#include "../../src/DebugConfig.h" // For LOG_BOOT

UIManager::UIManager(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

void UIManager::render(const UIRenderProps& props) {
    // Draw Top Screen
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    if (display_top) {
        display_top->clearDisplay();
        drawOledContent(display_top, props.oled_top_props);
        display_top->display();
    }

    // Draw Middle Screen
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    Adafruit_SSD1306* display_middle = _displayManager.getDisplay(1);
    if (display_middle) {
        display_middle->clearDisplay();
        drawOledContent(display_middle, props.oled_middle_props);
        display_middle->display();
    }

    // Draw Bottom Screen
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    if (display_bottom) {
        display_bottom->clearDisplay();
        drawOledContent(display_bottom, props.oled_bottom_props);
        display_bottom->display();
    }
}

void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) {
        return;
    }

    MenuBlock::draw(display, props.menu_props);

    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    if (!props.line1.empty()) {
        display->setCursor(0, 0);
        display->print(props.line1.c_str());
    }
    if (!props.line2.empty()) {
        display->setCursor(0, 10);
        display->print(props.line2.c_str());
    }
    if (!props.line3.empty()) {
        display->setCursor(0, 20);
        display->print(props.line3.c_str());
    }
}