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

#define GT_SAMPLE_COUNT 512

class GuidedTuningEngine {
public:
    GuidedTuningEngine();
    ~GuidedTuningEngine();

    bool proposeSettings(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager, AutoTuningScreen& progressScreen);

private:
    // Internal stages of the heuristic algorithm
    bool captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen);
    bool captureLongSignalForLF(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, AutoTuningScreen& progressScreen, const char* filepath);
    void analyzeSignal(PBiosContext& context, const std::vector<double>& signal_to_analyze);
    void deriveHfParameters(PBiosContext& context);

    // --- DEFINITIVE REFACTOR: Simplified signature ---
    void deriveLfParameters(PBiosContext& context, AdcManager& adcManager, SdManager& sdManager, StateManager* stateManager);
    
    void applyRefinement(PI_Filter* currentFilter, const PI_Filter& idealParams);

    // FFT-related members
    arduinoFFT* _FFT; 
    double* _fftReal;
    double* _fftImag;
};

#endif // GUIDED_TUNING_ENGINE_H