// File Path: /src/ui/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
#include "DebugConfig.h"

// ... (constructor and render() are unchanged) ...
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
    
    drawOledContent(display_top, props.oled_top_props);
    drawOledContent(display_middle, props.oled_middle_props);
    drawOledContent(display_bottom, props.oled_bottom_props);
    
    ButtonBlock::draw(_displayManager, props.button_props);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    
    if (display_top) display_top->display();
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    
    if (display_middle) display_middle->display();
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    
    if (display_bottom) display_bottom->display();
}


/**
 * @brief --- CRITICAL FIX: The custom drawing logic is now simplified and robust. ---
 */
void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) {
        return;
    }
    
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    // Standard multi-line text drawing for all screens
    if (!props.line1.empty()) {
        display->setCursor(4, 2);
        display->print(props.line1.c_str());
    }
    if (!props.line2.empty()) {
        display->setCursor(4, 12);
        display->print(props.line2.c_str());
    }

    // Special case for drawing an inverse box, now cleanly separated.
    // The ParameterEditScreen uses line3 to pass the value for the box.
    if (!props.line3.empty() && !props.menu_props.is_enabled) { 
        int16_t x1, y1;
        uint16_t w, h;
        display->getTextBounds(props.line3.c_str(), 0, 0, &x1, &y1, &w, &h);
        int value_x = 4;
        int value_y = 22; // Draw on its own line for clarity
        display->fillRect(value_x - 2, value_y - 2, w + 4, h + 4, SSD1306_WHITE);
        display->setCursor(value_x, value_y);
        display->setTextColor(SSD1306_BLACK);
        display->print(props.line3.c_str());
    }

    MenuBlock::draw(display, props.menu_props);
    GraphBlock::draw(display, props.graph_props);
    ProgressBarBlock::draw(display, props.progress_bar_props);
}