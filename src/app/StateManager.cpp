// src/app/StateManager.cpp
// MODIFIED FILE

#include "app/StateManager.h"
#include "DebugMacros.h"
#include <Arduino.h> 

StateManager::StateManager() {}

StateManager::~StateManager() {
    // <<< FIX: Changed to a C++11-compatible for-loop to resolve compiler warning.
    for (std::map<ScreenState, Screen*>::iterator it = _screens.begin(); it != _screens.end(); ++it) {
        delete it->second;
    }
    _screens.clear();
}

void StateManager::begin() {
    LOG_MANAGER("StateManager initialized.\n");
    // Ensure there's at least one screen and transition to it.
    if (!_screens.empty() && _screens.count(_currentState)) {
        changeState(_currentState);
    }
}

void StateManager::addScreen(ScreenState state, Screen* screen) {
    _screens[state] = screen;
}

void StateManager::changeState(ScreenState new_state) {
    if (_activeScreen) {
        _activeScreen->onExit();
    }

    if (_screens.count(new_state)) {
        _currentState = new_state;
        _activeScreen = _screens[new_state];
        _activeScreen->onEnter(this);
        LOG_MANAGER("Changed screen state to %d\n", (int)new_state);
    } else {
        LOG_MAIN("[SM_ERROR] Attempted to change to an unknown screen state: %d\n", (int)new_state);
    }
}

Screen* StateManager::getActiveScreen() {
    return _activeScreen;
}