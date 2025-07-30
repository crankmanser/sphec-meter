// File Path: /lib/UIManager/src/UIManager.h
// MODIFIED FILE

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "DisplayManager.h"
#include "blocks/MenuBlock.h"

/**
 * @struct OledProps
 * @brief A structure to hold the rendering properties for a single OLED screen.
 * It contains properties for all possible UI blocks that can be drawn on it.
 */
struct OledProps {
    std::string line1;
    std::string line2;
    std::string line3;
    MenuBlockProps menu_props;
    // Props for other blocks like GraphBlock will be added here later.
};

/**
 * @struct UIRenderProps
 * @brief The "Shared Canvas". A single, comprehensive data structure that
 * describes the entire UI for one frame.
 *
 * This is the core of the declarative UI. A screen's job is to populate this
 * struct. The UIManager's job is to render it.
 */
struct UIRenderProps {
    OledProps oled_top_props;
    OledProps oled_middle_props;
    OledProps oled_bottom_props;
    // Props for global elements like status bars will be added here later.
};

/**
 * @class UIManager
 * @brief The central, stateless rendering engine for the UI.
 *
 * This cabinet takes a UIRenderProps data structure and translates it
 * into a series of drawing commands sent to the DisplayManager. It is the
 * heart of the "Canvas" core engine and holds no state of its own.
 */
class UIManager {
public:
    /**
     * @brief Constructor for the UIManager.
     * @param displayManager A reference to the low-level DisplayManager.
     */
    UIManager(DisplayManager& displayManager);

    /**
     * @brief Renders the entire UI for one frame based on the provided data.
     * @param props The UIRenderProps structure describing what to draw.
     */
    void render(const UIRenderProps& props);

private:
    /**
     * @brief Helper function to render the content for a single OLED.
     * @param display A pointer to the Adafruit_SSD1306 object to draw on.
     * @param props The properties for that specific screen.
     */
    // --- FIX: Updated the function signature to match the implementation ---
    void drawOledContent(Adafruit_SSD1306* display, const OledProps& props);

    DisplayManager& _displayManager;
};

#endif // UI_MANAGER_H