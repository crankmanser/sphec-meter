// src/app/EncoderTask.cpp
// MODIFIED FILE
#include "app/EncoderTask.h"
#include "managers/io/EncoderManager.h"
#include "DebugMacros.h"

// Quadrature Encoder State Machine Table from legacy code
const int8_t qem_decode_table[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

// <<< MODIFIED: Speed Engine constants tuned to be much less sensitive. >>>
// A full turn of the 600 P/R encoder will now result in roughly 10 UI steps.
const int PULSES_PER_STEP_FINE = 240;
const int PULSES_PER_STEP_MEDIUM = 60;
const int PULSES_PER_STEP_FAST = 15;
const int SPEED_THRESHOLD_MEDIUM = 20;
const int SPEED_THRESHOLD_FAST = 40;

static portMUX_TYPE encoderTaskMux = portMUX_INITIALIZER_UNLOCKED;

void encoderTask(void* pvParameters) {
    LOG_TASK("Encoder Task started.\n");

    uint8_t last_ab_state = 0;
    long raw_pulses = 0;
    int accumulated_pulses = 0;

    for (;;) {
        uint8_t current_state;
        if (xQueueReceive(EncoderManager::_event_queue, &current_state, portMAX_DELAY)) {
            
            raw_pulses += qem_decode_table[(last_ab_state << 2) | current_state];
            last_ab_state = current_state;

            if (raw_pulses == 0) continue;

            accumulated_pulses += raw_pulses;
            long turn_speed = abs(raw_pulses);
            raw_pulses = 0;

            int effective_pulses_per_step;
            if (turn_speed < SPEED_THRESHOLD_MEDIUM) {
                effective_pulses_per_step = PULSES_PER_STEP_FINE;
            } else if (turn_speed < SPEED_THRESHOLD_FAST) {
                effective_pulses_per_step = PULSES_PER_STEP_MEDIUM;
            } else {
                effective_pulses_per_step = PULSES_PER_STEP_FAST;
            }

            int ui_steps = accumulated_pulses / effective_pulses_per_step;
            if (ui_steps != 0) {
                accumulated_pulses %= effective_pulses_per_step;
                portENTER_CRITICAL(&encoderTaskMux);
                EncoderManager::_ui_steps += ui_steps;
                portEXIT_CRITICAL(&encoderTaskMux);
            }
        }
    }
}