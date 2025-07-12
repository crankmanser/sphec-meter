// src/app/common/App_types.h
#pragma once

// Defines all possible screens in the application.
// Used by the StateManager to identify which screen to activate.
enum class ScreenState {
    SCREEN_HOME,
    SCREEN_MAIN_MENU,
    SCREEN_LIVE_READING,
    SCREEN_CALIBRATION_WIZARD
    // ... more screens to be added
};

// Defines the types of input events the UI can process.
enum class InputEventType {
    BTN_TOP_PRESS,
    BTN_MIDDLE_PRESS,
    BTN_BOTTOM_PRESS,
    BTN_TOP_HELD,
    BTN_MIDDLE_HELD,
    BTN_BOTTOM_HELD,
    ENCODER_INCREMENT,
    ENCODER_DECREMENT
};

struct InputEvent {
    InputEventType type;
    int value; // e.g., for encoder change amount
};