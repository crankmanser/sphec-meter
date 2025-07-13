// src/managers/io/EncoderManager.h
// MODIFIED FILE
#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class EncoderManager {
public:
    EncoderManager(); // Constructor no longer takes pins
    void begin();
    int getChange(); // Gets the change processed by the EncoderTask

private:
    // --- ISR-related static members ---
    static void IRAM_ATTR isr(); 
    
    // --- RTOS Communication ---
    // The ISR sends raw pin states to this queue.
    static QueueHandle_t _event_queue;
    // The EncoderTask calculates the final steps and stores them here.
    static volatile int _ui_steps;

    // Friend class declaration to allow EncoderTask to access private members
    friend void encoderTask(void* pvParameters);
};