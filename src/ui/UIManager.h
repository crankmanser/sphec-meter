// File Path: /src/ui/UIManager.h
// NEW FILE

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "DisplayManager.h"
#include "blocks/MenuBlock.h"
#include "blocks/ButtonBlock.h"

struct OledProps {
    std::string line1;
    std::string line2;
    std::string line3;
    MenuBlockProps menu_props;
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