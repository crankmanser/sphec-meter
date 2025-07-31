// File Path: /src/ui/blocks/ButtonBlock.h
// NEW FILE

#ifndef BUTTON_BLOCK_H
#define BUTTON_BLOCK_H

#include <string>
#include "DisplayManager.h" // Needs access to the DisplayManager
#include "ProjectConfig.h"  // Needs access to TCA channel definitions

/**
 * @struct ButtonBlockProps
 * @brief Holds the text labels for the three main UI buttons.
 */
struct ButtonBlockProps {
    std::string back_text;  // For the top button (Back/Up)
    std::string enter_text; // For the middle button (Enter/Select)
    std::string down_text;  // For the bottom button (Down/Next)
};

/**
 * @class ButtonBlock
 * @brief A stateless, reusable UI component for drawing button prompts.
 *
 * This block enforces the standard layout rule: a button's prompt is
 * displayed on the OLED screen directly opposite to it.
 */
class ButtonBlock {
public:
    static void draw(DisplayManager& displayManager, const ButtonBlockProps& props);
};

#endif // BUTTON_BLOCK_H