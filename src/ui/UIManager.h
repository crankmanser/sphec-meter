// File Path: /src/ui/UIManager.h
// MODIFIED FILE

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "DisplayManager.h"
#include "blocks/MenuBlock.h"
#include "blocks/ButtonBlock.h"
#include "blocks/GraphBlock.h"
#include "blocks/ProgressBarBlock.h"

struct OledProps {
    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4; 
    MenuBlockProps menu_props;
    GraphBlockProps graph_props;
    ProgressBarProps progress_bar_props;
};

struct UIRenderProps {
    OledProps oled_top_props;
    OledProps oled_middle_props;
    OledProps oled_bottom_props;
    ButtonBlockProps button_props;
};

class UIManager {
public:
    UIManager(DisplayManager& displayManager);
    void render(const UIRenderProps& props);

private:
    // --- FIX: This declaration is now correct and matches the definition ---
    void drawOledContent(Adafruit_SSD1306* display, const OledProps& props);
    DisplayManager& _displayManager;
};

#endif // UI_MANAGER_H