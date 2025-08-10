// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.h
// MODIFIED FILE

#ifndef GUIDED_TUNING_ENGINE_H
#define GUIDED_TUNING_ENGINE_H

#include "FilterManager.h"
#include "AdcManager.h"
#include "SdManager.h"
#include "pBiosContext.h"
#include "ui/screens/AutoTuningScreen.h"
#include "ui/StateManager.h"
#include <vector>
#include <arduinoFFT.h>

// --- DEFINITIVE FIX: Reduced sample count for safe RAM-based capture ---
#define GT_SAMPLE_COUNT 256 // Approx. 2KB per capture, well within safe RAM limits

class GuidedTuningEngine {
public:
    GuidedTuningEngine();
    ~GuidedTuningEngine();

    bool proposeSettings(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager, AutoTuningScreen& progressScreen);

private:
    // --- DEFINITIVE REFACTOR: Internal stages are now RAM-based ---
    bool captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen);
    void analyzeSignal(PBiosContext& context, const std::vector<double>& signal_to_analyze);
    void deriveHfParameters(PBiosContext& context);
    void deriveLfParameters(PBiosContext& context); // LF stage no longer needs extra dependencies
    
    void applyRefinement(PI_Filter* currentFilter, const PI_Filter& idealParams);

    // FFT-related members
    arduinoFFT* _FFT; 
    double* _fftReal;
    double* _fftImag;
};

#endif // GUIDED_TUNING_ENGINE_H