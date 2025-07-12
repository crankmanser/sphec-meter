// src/presentation/UIManager.h
// MODIFIED FILE
#pragma once

#include "presentation/DisplayManager.h"
#include "presentation/common/UIRenderProps.h"

/**
 * @class UIManager
 * @brief The central, stateless rendering engine for the UI.
 *
 * This class is the heart of the "Canvas" core. Its only job is to
 * take a declarative UIRenderProps data structure and translate it
 * into a series of drawing commands sent to the DisplayManager.
 * It holds no state of its own.
 */
class UIManager {
public:
    UIManager(DisplayManager& displayManager);
    void begin();

    // Renders the entire UI based on the provided data structure.
    void render(const UIRenderProps& props);

private:
    DisplayManager& _displayManager;

    // Private helper methods for drawing specific UI components.
    void drawTopStatusBar(const TopStatusProps& props);
    void drawStateStack(const StateStackProps& props);
    void drawButtonPrompts(const ButtonPrompt& props);
    // <<< MODIFIED: Signature updated to accept the show_top_bar flag.
    void drawOledContent(OLED_ID oled, const OledProps& props, bool show_top_bar);

    // Private helper for drawing icons from a bitmap atlas.
    void drawIcon(OLED_ID oled, int16_t x, int16_t y, Icon_ID icon);
};