// File Path: /lib/InputManager/src/InputManager.cpp
// MODIFIED FILE

#include "InputManager.h"
// --- FIX: The include for ProjectConfig.h is no longer needed here ---

volatile uint8_t InputManager::_encoder_raw_state = 0;
const int8_t InputManager::QEM_DECODE_TABLE[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

InputManager::InputManager() :
    _btn_top(BTN_BACK_PIN),
    _btn_middle(BTN_ENTER_PIN),
    _btn_bottom(BTN_DOWN_PIN),
    _task_handle(NULL),
    _event_queue(NULL)
{}

bool InputManager::begin() {
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);

    _event_queue = xQueueCreate(10, sizeof(InputEvent));
    if (_event_queue == NULL) {
        return false;
    }

    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);

    BaseType_t result = xTaskCreatePinnedToCore(
        inputTask, "InputTask", 2048, this, TASK_PRIORITY_HIGH, &_task_handle, 1
    );

    return result == pdPASS;
}
// ... (rest of InputManager.cpp is unchanged) ...
void IRAM_ATTR InputManager::encoderISR() { _encoder_raw_state = (digitalRead(ENCODER_PIN_B) << 1) | digitalRead(ENCODER_PIN_A); }
void InputManager::inputTask(void* pvParameters) { static_cast<InputManager*>(pvParameters)->run(); }
void InputManager::run() {
    uint8_t last_encoder_state = 0;
    long accumulated_pulses = 0;
    for (;;) {
        uint32_t now = millis();
        bool top_pressed = (digitalRead(_btn_top.pin) == HIGH);
        bool middle_pressed = (digitalRead(_btn_middle.pin) == HIGH);
        bool bottom_pressed = (digitalRead(_btn_bottom.pin) == HIGH);
        if (top_pressed && !_btn_top.last_state && (now - _btn_top.last_press_time > DEBOUNCE_DELAY_MS)) {
            InputEvent event = {InputEventType::BTN_TOP_PRESS, 0};
            xQueueSend(_event_queue, &event, 0);
            _btn_top.last_press_time = now;
        }
        if (middle_pressed && !_btn_middle.last_state && (now - _btn_middle.last_press_time > DEBOUNCE_DELAY_MS)) {
            InputEvent event = {InputEventType::BTN_MIDDLE_PRESS, 0};
            xQueueSend(_event_queue, &event, 0);
            _btn_middle.last_press_time = now;
        }
        if (bottom_pressed && !_btn_bottom.last_state && (now - _btn_bottom.last_press_time > DEBOUNCE_DELAY_MS)) {
            InputEvent event = {InputEventType::BTN_BOTTOM_PRESS, 0};
            xQueueSend(_event_queue, &event, 0);
            _btn_bottom.last_press_time = now;
        }
        _btn_top.last_state = top_pressed;
        _btn_middle.last_state = middle_pressed;
        _btn_bottom.last_state = bottom_pressed;
        uint8_t current_encoder_state = _encoder_raw_state;
        if (current_encoder_state != last_encoder_state) {
            accumulated_pulses += QEM_DECODE_TABLE[(last_encoder_state << 2) | current_encoder_state];
            last_encoder_state = current_encoder_state;
            if (abs(accumulated_pulses) >= 4) {
                InputEventType type = (accumulated_pulses > 0) ? InputEventType::ENCODER_INCREMENT : InputEventType::ENCODER_DECREMENT;
                InputEvent event = {type, 0};
                xQueueSend(_event_queue, &event, 0);
                accumulated_pulses = 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
bool InputManager::getEvent(InputEvent& event, uint32_t timeout_ms) {
    if (_event_queue == NULL) return false;
    return xQueueReceive(_event_queue, &event, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}