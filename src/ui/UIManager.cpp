// File Path: /src/ui/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
#include "DebugConfig.h"
#include "ProjectConfig.h" // Include for OLED TCA channel definitions

UIManager::UIManager(DisplayManager& displayManager) : _displayManager(displayManager) {}

void UIManager::render(const UIRenderProps& props) {
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->clearDisplay();

    Adafruit_SSD1306* display_middle = _displayManager.getDisplay(1);
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->clearDisplay();

    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->clearDisplay();
    
    // Draw content for each OLED
    drawOledContent(display_top, props.oled_top_props);
    drawOledContent(display_middle, props.oled_middle_props);
    drawOledContent(display_bottom, props.oled_bottom_props);
    
    // Draw the button prompts across all three displays
    ButtonBlock::draw(_displayManager, props.button_props);
    
    // Refresh the physical displays
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->display();
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->display();
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->display();
}

void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) return;
    
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    if (!props.line1.empty()) {
        if (props.line1.rfind("Value: ", 0) == 0) {
            int16_t x1, y1;
            uint16_t w, h;
            display->getTextBounds(props.line1.c_str(), 0, 0, &x1, &y1, &w, &h);
            int rect_x = (SCREEN_WIDTH - w) / 2 - 2;
            int rect_y = 2;
            display->fillRect(rect_x, rect_y, w + 4, h + 4, SSD1306_WHITE);
            display->setCursor(rect_x + 2, rect_y + 2);
            display->setTextColor(SSD1306_BLACK);
            display->print(props.line1.c_str());
            display->setTextColor(SSD1306_WHITE);
        } else {
            display->setCursor(2, 2);
            display->print(props.line1.c_str());
        }
    }
    if (!props.line2.empty()) {
        if (props.line2.rfind("Value: ", 0) == 0) {
            display->setCursor(2, 16); // Adjusted Y for new layout
            display->print(props.line2.c_str());
        } else {
            display->setCursor(2, 12);
            display->print(props.line2.c_str());
        }
    }
    if (!props.line3.empty()) {
        if (props.line3[0] == '>') {
             int16_t x1, y1;
            uint16_t w, h;
            display->getTextBounds(props.line3.c_str(), 0, 0, &x1, &y1, &w, &h);
            int rect_x = (128 - w) / 2;
            int rect_y = 50; // Adjusted Y for new layout
            display->fillRect(rect_x - 2, rect_y, w + 4, h + 4, SSD1306_WHITE);
            display->setCursor(rect_x, rect_y + 2);
            display->setTextColor(SSD1306_BLACK);
            display->print(props.line3.c_str());
            display->setTextColor(SSD1306_WHITE);
        } else {
            display->setCursor(2, 22);
            display->print(props.line3.c_str());
        }
    }

    if (props.graph_props.is_enabled) {
        GraphBlock::draw(display, props.graph_props);
    }

    if (props.progress_bar_props.is_enabled) {
        ProgressBarBlock::draw(display, props.progress_bar_props);
    }

    if (props.menu_props.is_enabled) {
        if (!props.line1.empty() || !props.line2.empty()) {
            display->drawFastHLine(0, 28, 128, SSD1306_WHITE); // Adjusted Y for new layout
        }
        MenuBlock::draw(display, props.menu_props);
    }
}