// File Path: /lib/InputManager/src/InputManager.h
// MODIFIED FILE

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
// --- FIX: This will now be found correctly thanks to library.json ---
#include "ProjectConfig.h"

enum class InputEventType {
    BTN_TOP_PRESS,
    BTN_MIDDLE_PRESS,
    BTN_BOTTOM_PRESS,
    ENCODER_INCREMENT,
    ENCODER_DECREMENT
};

struct InputEvent {
    InputEventType type;
    int value;
};

class InputManager {
public:
    InputManager();
    bool begin();
    bool getEvent(InputEvent& event, uint32_t timeout_ms = 0);

private:
    static void inputTask(void* pvParameters);
    static void IRAM_ATTR encoderISR();
    void run();

    struct Button {
        const uint8_t pin;
        bool last_state;
        uint32_t last_press_time;
        Button(uint8_t p) : pin(p), last_state(false), last_press_time(0) {}
    };
    Button _btn_top;
    Button _btn_middle;
    Button _btn_bottom;

    static volatile uint8_t _encoder_raw_state;
    static const int8_t QEM_DECODE_TABLE[16];

    TaskHandle_t _task_handle;
    QueueHandle_t _event_queue;

    static constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
};

#endif // INPUT_MANAGER_H