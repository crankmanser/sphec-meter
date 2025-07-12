// src/presentation/UIManager.cpp
// MODIFIED FILE

#include "presentation/UIManager.h"
#include "DebugMacros.h"
#include "presentation/resources/icons.h"

UIManager::UIManager(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

void UIManager::begin() {
    LOG_MANAGER("UIManager initialized.\n");
}

void UIManager::render(const UIRenderProps& props) {
    _displayManager.clearAll();

    if (props.show_top_bar) {
        drawTopStatusBar(props.top_status_props);
    }
    drawStateStack(props.state_stack_props);
    
    // For this test, we won't draw main content to keep the focus on the status areas.
    // if (props.oled_top_props.is_dirty) { ... }
    
    drawButtonPrompts(props.button_prompts);
    _displayManager.displayAll();
}

void UIManager::drawTopStatusBar(const TopStatusProps& props) {
    Adafruit_SSD1306* oled1 = _displayManager.getDisplay(OLED_ID::OLED_TOP);
    _displayManager.selectOLED(OLED_ID::OLED_TOP);
    oled1->fillRect(0, 0, 128, 18, SSD1306_WHITE);

    Adafruit_SSD1306* oled2 = _displayManager.getDisplay(OLED_ID::OLED_MIDDLE);
    _displayManager.selectOLED(OLED_ID::OLED_MIDDLE);
    oled2->fillRect(0, 0, 128, 18, SSD1306_WHITE);

    // Draw icons onto the bar
    drawIcon(OLED_ID::OLED_TOP, 2, 1, props.ph_probe_icon);
    drawIcon(OLED_ID::OLED_TOP, 20, 1, props.ph_kpi_icon);
    drawIcon(OLED_ID::OLED_TOP, 40, 1, props.ec_probe_icon);
    drawIcon(OLED_ID::OLED_TOP, 58, 1, props.ec_kpi_icon);
    
    drawIcon(OLED_ID::OLED_MIDDLE, 2, 1, props.bus_3v3_icon);
    drawIcon(OLED_ID::OLED_MIDDLE, 20, 1, props.bus_5v_icon);
    drawIcon(OLED_ID::OLED_MIDDLE, 90, 1, props.sd_card_icon);
    drawIcon(OLED_ID::OLED_MIDDLE, 110, 1, props.wifi_icon);

    _displayManager.selectOLED(OLED_ID::OLED_TOP);
    oled1->setTextSize(1);
    oled1->setTextColor(SSD1306_BLACK); // Text on white background
    oled1->setCursor(70, 1);
    oled1->print(props.time_text.c_str());

    _displayManager.selectOLED(OLED_ID::OLED_MIDDLE);
    oled2->setTextSize(1);
    oled2->setTextColor(SSD1306_BLACK);
    oled2->setCursor(40, 1);
    oled2->print(props.date_text.c_str());
}

void UIManager::drawStateStack(const StateStackProps& props) {
    Adafruit_SSD1306* display = _displayManager.getDisplay(OLED_ID::OLED_BOTTOM);
    _displayManager.selectOLED(OLED_ID::OLED_BOTTOM);
    display->fillRect(0, 0, 20, 64, SSD1306_WHITE);
    drawIcon(OLED_ID::OLED_BOTTOM, 2, 2, props.icon1);
    drawIcon(OLED_ID::OLED_BOTTOM, 2, 22, props.icon2);
    drawIcon(OLED_ID::OLED_BOTTOM, 2, 42, props.icon3);
}

void UIManager::drawButtonPrompts(const ButtonPrompt& props) {
    // This logic is now part of the main screen content area on OLED_BOTTOM
    Adafruit_SSD1306* display = _displayManager.getDisplay(OLED_ID::OLED_BOTTOM);
    _displayManager.selectOLED(OLED_ID::OLED_BOTTOM);
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(24, 56);
    display->print(props.top_button_text.c_str());
}

void UIManager::drawIcon(OLED_ID oled, int16_t x, int16_t y, Icon_ID icon) {
    Adafruit_SSD1306* display = _displayManager.getDisplay(oled);
    _displayManager.selectOLED(oled);
    display->drawBitmap(x, y, icon_bitmaps[(int)icon], 16, 16, SSD1306_BLACK, SSD1306_WHITE);
}

// Omitted drawOledContent for brevity as it's not used in this test
void UIManager::drawOledContent(OLED_ID oled, const OledProps& props) {}