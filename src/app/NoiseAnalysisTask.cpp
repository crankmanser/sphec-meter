// src/app/NoiseAnalysisTask.cpp
// NEW FILE
#include "NoiseAnalysisTask.h"
#include "managers/diagnostics/NoiseAnalysisManager.h"
#include "DebugMacros.h"

void noiseAnalysisTask(void* pvParameters) {
    if (pvParameters == nullptr) {
        LOG_MAIN("[NA_TASK_ERROR] Task parameters are null. Deleting task.\n");
        vTaskDelete(NULL);
        return;
    }

    NoiseAnalysisParams* params = static_cast<NoiseAnalysisParams*>(pvParameters);
    LOG_TASK("Noise Analysis Task started for sensor %d.\n", (int)params->sensor);

    if (params->manager) {
        params->manager->performAnalysis(params->sensor);
    }

    // Signal completion to the screen
    if (params->completion_flag) {
        *(params->completion_flag) = true;
    }

    LOG_TASK("Noise Analysis Task finished. Deleting task.\n");

    // The task is done, so it deletes itself to free resources.
    vTaskDelete(NULL);
}