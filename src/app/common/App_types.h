// src/app/common/App_types.h
// MODIFIED FILE
#pragma once

// Defines all possible screens in the application.
// Used by the StateManager to identify which screen to activate.
enum class ScreenState {
    SCREEN_HOME, // This will now be the main menu
    SCREEN_MAIN_MENU, // Redundant but good for clarity
    SCREEN_LIVE_READING,
    SCREEN_DIAGNOSTICS_MENU, // <<< ADDED
    SCREEN_NOISE_ANALYSIS,   // <<< ADDED
    SCREEN_SETTINGS_MENU,    // <<< ADDED
    SCREEN_CALIBRATION_WIZARD
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