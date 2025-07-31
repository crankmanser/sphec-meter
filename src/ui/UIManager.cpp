// File Path: /src/ui/UIManager.cpp
// MODIFIED FILE

#include "UIManager.h"
#include "DebugConfig.h"

UIManager::UIManager(DisplayManager& displayManager) :
    _displayManager(displayManager)
{}

/**
 * @brief Renders the entire UI for one frame.
 * This function clears all displays, draws all screen-specific content,
 * draws all global components (like button prompts), and then pushes
 * the completed buffers to the physical screens.
 */
void UIManager::render(const UIRenderProps& props) {
    // --- STAGE 1: Clear all display buffers in memory ---
    Adafruit_SSD1306* display_top = _displayManager.getDisplay(0);
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->clearDisplay();

    Adafruit_SSD1306* display_middle = _displayManager.getDisplay(1);
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->clearDisplay();

    Adafruit_SSD1306* display_bottom = _displayManager.getDisplay(2);
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->clearDisplay();
    
    // --- STAGE 2: Draw all content to the buffers ---
    drawOledContent(display_top, props.oled_top_props);
    drawOledContent(display_middle, props.oled_middle_props);
    drawOledContent(display_bottom, props.oled_bottom_props);
    ButtonBlock::draw(_displayManager, props.button_props);

    // --- STAGE 3: Push all buffers to the physical screens ---
    _displayManager.selectTCAChannel(OLED3_TCA_CHANNEL);
    if (display_top) display_top->display();
    
    _displayManager.selectTCAChannel(OLED2_TCA_CHANNEL);
    if (display_middle) display_middle->display();
    
    _displayManager.selectTCAChannel(OLED1_TCA_CHANNEL);
    if (display_bottom) display_bottom->display();
}

void UIManager::drawOledContent(Adafruit_SSD1306* display, const OledProps& props) {
    if (!display) {
        return;
    }

    MenuBlock::draw(display, props.menu_props);

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
