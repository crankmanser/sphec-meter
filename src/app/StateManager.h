// src/app/StateManager.h
#pragma once

#include "presentation/screens/Screen.h"
#include "app/common/App_types.h"
#include <map>

/**
 * @class StateManager
 * @brief Manages the active UI screen and transitions between them.
 *
 * This cabinet owns all the Screen objects and holds a pointer to the
 * currently active screen. It provides a single point of control for
 * navigating through the application's UI.
 */
class StateManager {
public:
    StateManager();
    ~StateManager();

    void begin();
    void addScreen(ScreenState state, Screen* screen);
    void changeState(ScreenState new_state);
    Screen* getActiveScreen();

private:
    std::map<ScreenState, Screen*> _screens;
    Screen* _activeScreen = nullptr;
    ScreenState _currentState = ScreenState::SCREEN_HOME;
};