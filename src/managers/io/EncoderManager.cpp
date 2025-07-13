// src/managers/io/EncoderManager.cpp
// MODIFIED FILE
#include "EncoderManager.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

// Initialize static members
QueueHandle_t EncoderManager::_event_queue = nullptr;
volatile int EncoderManager::_ui_steps = 0;

// <<< FIX: Define a spinlock for this file's critical section >>>
static portMUX_TYPE encoderManagerMux = portMUX_INITIALIZER_UNLOCKED;

EncoderManager::EncoderManager() {}

void IRAM_ATTR EncoderManager::isr() {
    uint8_t pin_a = digitalRead(ENCODER_A_PIN);
    uint8_t pin_b = digitalRead(ENCODER_B_PIN);
    uint8_t state = (pin_b << 1) | pin_a;
    xQueueSendFromISR(_event_queue, &state, NULL);
}

void EncoderManager::begin() {
    _event_queue = xQueueCreate(32, sizeof(uint8_t));

    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), isr, CHANGE);
    LOG_MANAGER("Event-driven EncoderManager initialized.\n");
}

int EncoderManager::getChange() {
    // <<< FIX: Pass the spinlock to the critical section macros >>>
    portENTER_CRITICAL(&encoderManagerMux);
    int steps = _ui_steps;
    _ui_steps = 0;
    portEXIT_CRITICAL(&encoderManagerMux);
    
    return steps;
}