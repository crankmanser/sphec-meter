// File Path: /src/ui/UIManager.h
// MODIFIED FILE

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "DisplayManager.h"
#include "blocks/MenuBlock.h"
#include "blocks/ButtonBlock.h"
#include "blocks/GraphBlock.h"
#include "blocks/ProgressBarBlock.h" // <<< NEW: Include the new ProgressBarBlock

struct OledProps {
    std::string line1;
    std::string line2;
    std::string line3;
    MenuBlockProps menu_props;
    GraphBlockProps graph_props;
    ProgressBarProps progress_bar_props; // <<< NEW: Add progress bar properties
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
    void drawOledContent(Adafruit_SSD1306* display, const OledProps& props);
    DisplayManager& _displayManager;
};

#endif // UI_MANAGER_H