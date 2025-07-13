// src/app/EncoderTask.h
// NEW FILE
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the high-priority Encoder RTOS task.
 *
 * This task's only job is to wait for events from the encoder's ISR queue,
 * process them using the robust legacy "Speed Engine" logic, and make the
 * final UI step count available to the rest of the system.
 *
 * @param pvParameters A pointer to task parameters (not used).
 */
void encoderTask(void* pvParameters);