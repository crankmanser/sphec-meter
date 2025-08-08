// File Path: /src/ui/StateManager.cpp
// MODIFIED FILE

#include "StateManager.h"

StateManager::StateManager() :
    _activeScreen(nullptr),
    _currentState(ScreenState::NONE),
    _uiRenderProps(nullptr)
{}

StateManager::~StateManager() {
    // --- DEFINITIVE FIX: Revert to a C++11 compatible iterator loop ---
    // This resolves the compiler warning about structured bindings.
    for (std::map<ScreenState, Screen*>::iterator it = _screens.begin(); it != _screens.end(); ++it) {
        delete it->second;
    }
    _screens.clear();
    if (_uiRenderProps) {
        delete _uiRenderProps;
        _uiRenderProps = nullptr;
    }
}

void StateManager::begin() {
    if (!_uiRenderProps) {
        _uiRenderProps = new UIRenderProps();
    }
}

void StateManager::addScreen(ScreenState state, Screen* screen) {
    if (_screens.find(state) == _screens.end()) {
        _screens[state] = screen;
    }
}

void StateManager::changeState(ScreenState new_state) {
    if (_activeScreen != nullptr) {
        _activeScreen->onExit();
    }
    auto it = _screens.find(new_state);
    if (it != _screens.end()) {
        _currentState = new_state;
        _activeScreen = it->second;
        _activeScreen->onEnter(this);
    } else {
        _activeScreen = nullptr;
        _currentState = ScreenState::NONE;
    }
}

Screen* StateManager::getActiveScreen() {
    return _activeScreen;
}

Screen* StateManager::getScreen(ScreenState state) {
    auto it = _screens.find(state);
    if (it != _screens.end()) {
        return it->second;
    }
    return nullptr;
}

ScreenState StateManager::getActiveScreenState() const {
    return _currentState;
}

UIRenderProps* StateManager::getUiRenderProps() {
    return _uiRenderProps;
}