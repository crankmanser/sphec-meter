// File Path: /src/ui/InputManager.h
// MODIFIED FILE

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

enum class InputEventType {
    BTN_BACK_PRESS,
    BTN_ENTER_PRESS,
    BTN_DOWN_PRESS,
    ENCODER_INCREMENT,
    ENCODER_DECREMENT
};

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
    // --- MIGRATED FROM LEGACY: The exact ISR implementation from the working code ---
    static void IRAM_ATTR encoderISR();
    static volatile long _encoder_raw_pulses;
    static volatile uint8_t _last_AB_state;
    static const int8_t _qem_decode_table[];
    
    long _accumulated_pulses;

    // --- Button debouncing state (unchanged) ---
    bool _back_pressed;
    bool _enter_pressed;
    bool _down_pressed;
    int _encoder_change;
    bool _back_last_state;
    uint32_t _back_last_debounce_time;
    bool _enter_last_state;
    uint32_t _enter_last_debounce_time;
    bool _down_last_state;
    uint32_t _down_last_debounce_time;
};

#endif // INPUT_MANAGER_H