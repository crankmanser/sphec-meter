// File Path: /src/pBiosContext.h
// MODIFIED FILE

#ifndef PBIOS_CONTEXT_H
#define PBIOS_CONTEXT_H

#include "FilterManager.h"
#include "AdcManager.h"
#include <string>
#include <vector>

/**
 * @brief --- DEFINITIVE REFACTOR: This struct is now the "float" for the tuning wizard. ---
 * It holds the state and intermediate data required to pass information between the
 * discrete stages of the auto-tuning process, which is now managed by a state
 * machine in the pBiosDataTask.
 */
struct PBiosContext {
    // --- Live Tuning State ---
    FilterManager* selectedFilter = nullptr;
    std::string selectedFilterName; 
    uint8_t selectedAdcIndex = 0;
    uint8_t selectedAdcInput = 0;

    // --- Auto-Tuner Wizard State ---
    // A snapshot of the filter parameters at the beginning of the tuning pass.
    PI_Filter hf_params_snapshot;
    PI_Filter lf_params_snapshot;
    
    // The raw signal data captured during the analysis stage.
    std::vector<double> captured_samples;

    // The results of the signal characterization.
    double raw_std_dev = 0.0;
    double peak_frequency = 0.0;
    
    /**
     * @brief --- NEW: The peak-to-peak amplitude of the signal. ---
     * This is a key metric used by the new "Characterization-Driven" heuristic
     * to determine the overall power and nature of the noise.
     */
    double pk_pk_amplitude = 0.0;
};

extern FilterManager phFilter;
extern FilterManager ecFilter;
extern FilterManager v3_3_Filter;
extern FilterManager v5_0_Filter;

#endif // PBIOS_CONTEXT_H