// src/app/NoiseAnalysisTask.h
// NEW FILE
#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief The main function for the one-shot Noise Analysis RTOS task.
 *
 * This task is created dynamically by the NoiseAnalysisScreen. Its only job
 * is to run the computationally expensive analysis without blocking the UI.
 * Once the analysis is complete, it signals the screen and deletes itself.
 *
 * @param pvParameters A pointer to a NoiseAnalysisParams struct.
 */
void noiseAnalysisTask(void* pvParameters);