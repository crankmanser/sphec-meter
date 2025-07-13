// src/presentation/UIManager.cpp
// MODIFIED FILE
#include "presentation/UIManager.h"
#include "DebugMacros.h"
#include "presentation/resources/icons.h"
#include "presentation/blocks/MenuBlock.h"
#include "presentation/blocks/GraphBlock.h" // <<< ADDED

// ... (Constructor and begin methods are unchanged)
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
    
    drawOledContent(OLED_ID::OLED_TOP, props.oled_top_props, props.show_top_bar);
    drawOledContent(OLED_ID::OLED_MIDDLE, props.oled_middle_props, props.show_top_bar);
    drawOledContent(OLED_ID::OLED_BOTTOM, props.oled_bottom_props, props.show_top_bar);
    
    drawButtonPrompts(props.button_prompts);
    _displayManager.displayAll();
}

// ... (drawTopStatusBar and drawStateStack are unchanged)
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
    oled1->setCursor(78, 5);
    oled1->print(props.time_text.c_str());

    _displayManager.selectOLED(OLED_ID::OLED_MIDDLE);
    oled2->setTextSize(1);
    oled2->setTextColor(SSD1306_BLACK);
    oled2->setCursor(40, 5);
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

// <<< MODIFIED: This function now draws graphical prompts in the specified locations >>>
void UIManager::drawButtonPrompts(const ButtonPrompt& props) {
    auto draw_single_prompt = [&](OLED_ID oled_id, const std::string& text) {
        if (text.empty()) return;
        Adafruit_SSD1306* display = _displayManager.getDisplay(oled_id);
        _displayManager.selectOLED(oled_id);
        
        int16_t x1, y1;
        uint16_t w, h;
        display->setTextSize(1);
        display->getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

        const int x_margin = 4;
        const int y_margin = 2;
        int rect_x = (display->width() - w) / 2 - x_margin;
        int rect_y = 54;
        int rect_w = w + (x_margin * 2);
        int rect_h = h + (y_margin * 2);

        display->fillRect(0, 54, 128, 10, SSD1306_BLACK); // Clear area
        display->fillRoundRect(rect_x, rect_y, rect_w, rect_h, 2, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK);
        display->setCursor(rect_x + x_margin, rect_y + y_margin + 1);
        display->print(text.c_str());
        display->setTextColor(SSD1306_WHITE);
    };

    // Draw prompts according to the new layout
    draw_single_prompt(OLED_ID::OLED_TOP, props.bottom_button_text);
    draw_single_prompt(OLED_ID::OLED_MIDDLE, props.middle_button_text);
    draw_single_prompt(OLED_ID::OLED_BOTTOM, props.top_button_text);
}

// ... (drawIcon is unchanged)
void UIManager::drawIcon(OLED_ID oled, int16_t x, int16_t y, Icon_ID icon) {
    if ((int)icon >= (int)Icon_ID::ICON_COUNT) return;

    Adafruit_SSD1306* display = _displayManager.getDisplay(oled);
    _displayManager.selectOLED(oled);
    display->drawBitmap(x, y, icon_bitmaps[(int)icon], 16, 16, SSD1306_BLACK, SSD1306_WHITE);
}

// <<< MODIFIED: This function now delegates to UI blocks >>>
void UIManager::drawOledContent(OLED_ID oled, const OledProps& props, bool show_top_bar) {
    Adafruit_SSD1306* display = _displayManager.getDisplay(oled);
    _displayManager.selectOLED(oled);
    
    int clear_y = show_top_bar ? 19 : 0;
    int clear_h = 53 - clear_y; 
    
    if (oled == OLED_ID::OLED_BOTTOM) {
         display->fillRect(21, clear_y, 107, clear_h, SSD1306_BLACK);
    } else {
         display->fillRect(0, clear_y, 128, clear_h, SSD1306_BLACK);
    }

    // --- Draw Blocks ---
    MenuBlock::draw(display, props.menu_props);
    GraphBlock::draw(display, props.graph_props);

    // --- Draw simple text lines (if any) ---
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    int y_offset = show_top_bar ? 22 : 4;
    int x_offset = (oled == OLED_ID::OLED_BOTTOM) ? 24 : 4;

    if (!props.line1.empty()) {
        display->setCursor(x_offset, y_offset);
        display->print(props.line1.c_str());
    }
    if (!props.line2.empty()) {
        display->setCursor(x_offset, y_offset + 10);
        display->print(props.line2.c_str());
    }
     if (!props.line3.empty()) {
        display->setCursor(x_offset, y_offset + 20);
        display->print(props.line3.c_str());
    }
}