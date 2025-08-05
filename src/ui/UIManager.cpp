// File Path: /src/ui/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
#include "DebugConfig.h"

UIManager::UIManager(DisplayManager& displayManager) : _displayManager(displayManager) {}

void UIManager::render(const UIRenderProps& props) {
    // ... (clear display logic is unchanged) ...
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
 * @brief --- DEFINITIVE FIX: Renders the new Status/Menu layout correctly. ---
 */
void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) return;
    
    display->setTextSize(1);
    display->setFont(nullptr);
    display->setTextColor(SSD1306_WHITE);

    GraphBlock::draw(display, props.graph_props);
    
    // --- Render logic for the Middle OLED ---
    if (props.menu_props.is_enabled) {
        // Status Area: Live calibrated value
        display->setCursor(2, 2);
        display->print(props.line1.c_str());
        
        // Status Area: Selected parameter's value with a tight inverse box
        if (!props.line2.empty() && props.line2 != "N/A") {
            std::string value_text = "Value: " + props.line2;
            int16_t x1, y1;
            uint16_t w, h;
            display->getTextBounds(value_text.c_str(), 0, 0, &x1, &y1, &w, &h);

            int rect_x = 2;
            int rect_y = 12;
            display->fillRect(rect_x, rect_y, w + 4, h + 4, SSD1306_WHITE);
            display->setCursor(rect_x + 2, rect_y + 2);
            display->setTextColor(SSD1306_BLACK);
            display->print(value_text.c_str());
            display->setTextColor(SSD1306_WHITE);
        }

        // Separator Line
        display->drawFastHLine(0, 26, 128, SSD1306_WHITE);

        // Menu Area (Bottom)
        MenuBlock::draw(display, props.menu_props);
    }
}