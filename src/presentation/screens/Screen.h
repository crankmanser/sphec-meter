// src/presentation/screens/Screen.h
// MODIFIED FILE
#pragma once

#include "presentation/common/UIRenderProps.h"
#include "app/common/App_types.h"

// Forward-declare dependencies to keep header clean
class StateManager;
struct AppContext;

/**
 * @class Screen
 * @brief An abstract base class for all UI screens.
 */
class Screen {
public:
    virtual ~Screen() {}
    // <<< MODIFIED: onEnter now accepts the AppContext >>>
    virtual void onEnter(StateManager* stateManager, AppContext* context) { 
        _stateManager = stateManager; 
        _appContext = context;
    };
    virtual void onExit() {};
    virtual void update() {};
    virtual void handleInput(const InputEvent& event) = 0;
    virtual UIRenderProps getRenderProps() = 0;

protected:
    StateManager* _stateManager = nullptr;
    // <<< ADDED: A pointer to the application context >>>
    AppContext* _appContext = nullptr;
};