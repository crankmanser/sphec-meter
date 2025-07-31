// File Path: /src/ui/InputManager.h
// NEW FILE

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

// Defines all possible user input events.
enum class InputEventType {
    BTN_BACK_PRESS,
    BTN_ENTER_PRESS,
    BTN_DOWN_PRESS,
    ENCODER_INCREMENT,
    ENCODER_DECREMENT
};

// A structure to hold a single, processed input event.
struct InputEvent {
    InputEventType type;
    int value = 0;
};

class InputManager {
public:
    InputManager();
    void begin();
    void update();

    bool wasBackPressed();
    bool wasEnterPressed();
    bool wasDownPressed();
    int getEncoderChange();

private:
    static void IRAM_ATTR encoderISR();

    bool _back_pressed;
    bool _enter_pressed;
    bool _down_pressed;
    int _encoder_change;

    // Internal state for debouncing
    bool _back_last_state;
    uint32_t _back_last_debounce_time;
    bool _enter_last_state;
    uint32_t _enter_last_debounce_time;
    bool _down_last_state;
    uint32_t _down_last_debounce_time;

    static volatile int _encoder_raw_pulses;
};

#endif // INPUT_MANAGER_H