// File Path: /src/ui/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
#include "DebugConfig.h"

UIManager::UIManager(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

/**
 * @brief Renders the entire UI for one frame.
 */
void UIManager::render(const UIRenderProps& props) {
    // Stage 1: Clear all display buffers
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->clearDisplay();

    Adafruit_SSD1306* display_middle = _displayManager.getDisplay(1);
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->clearDisplay();

    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->clearDisplay();
    
    // Stage 2: Draw all content to the buffers
    drawOledContent(display_top, props.oled_top_props);
    drawOledContent(display_middle, props.oled_middle_props);
    drawOledContent(display_bottom, props.oled_bottom_props);
    ButtonBlock::draw(_displayManager, props.button_props);

    // Stage 3: Push all buffers to the physical screens
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->display();
    
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->display();
    
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->display();
}

/**
 * @brief Draws the content for a single OLED screen.
 * This function now calls the appropriate block-drawing functions based
 * on the properties provided.
 */
void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) {
        return;
    }
    
    // --- NEW: Custom drawing for the bordered status area ---
    // This logic is specific to the LiveFilterTuningScreen, identified by a non-empty line2
    if (!props.line2.empty() && props.menu_props.is_enabled) {
        display->setTextSize(1);
        display->setFont(nullptr);

        // Draw the parameter name
        display->setCursor(4, 2);
        display->setTextColor(SSD1306_WHITE);
        display->print(props.line1.c_str());

        // Draw the value in an inverse box
        int16_t x1, y1;
        uint16_t w, h;
        display->getTextBounds(props.line2.c_str(), 0, 0, &x1, &y1, &w, &h);
        int value_x = 80;
        int value_y = 2;
        display->fillRect(value_x - 2, value_y - 2, w + 4, h + 4, SSD1306_WHITE);
        display->setCursor(value_x, value_y);
        display->setTextColor(SSD1306_BLACK);
        display->print(props.line2.c_str());

        // Draw the borderline
        display->drawFastHLine(0, 16, 128, SSD1306_WHITE);
    } 
    // --- Fallback for simple text rendering ---
    else {
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        if (!props.line1.empty()) {
            display->setCursor(4, 2);
            display->print(props.line1.c_str());
        }
        if (!props.line2.empty()) {
            display->setCursor(4, 12);
            display->print(props.line2.c_str());
        }
        if (!props.line3.empty()) {
            display->setCursor(4, 22);
            display->print(props.line3.c_str());
        }
    }


    // Call the draw methods for other UI blocks
    MenuBlock::draw(display, props.menu_props);
    GraphBlock::draw(display, props.graph_props);
}
