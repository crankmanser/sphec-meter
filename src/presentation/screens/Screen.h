// src/presentation/screens/Screen.h
#pragma once

#include "presentation/common/UIRenderProps.h"
#include "app/common/App_types.h"

class StateManager; // Forward declaration to avoid circular dependency

/**
 * @class Screen
 * @brief An abstract base class for all UI screens.
 *
 * This class defines the "contract" that all screens must adhere to.
 * A screen is a self-contained module that handles input and generates
 * render properties, but it does not draw anything itself.
 */
class Screen {
public:
    virtual ~Screen() {}
    virtual void onEnter(StateManager* stateManager) { _stateManager = stateManager; };
    virtual void onExit() {};
    virtual void update() {}; // For continuous updates
    virtual void handleInput(const InputEvent& event) = 0;
    virtual UIRenderProps getRenderProps() = 0;

protected:
    StateManager* _stateManager = nullptr;
};