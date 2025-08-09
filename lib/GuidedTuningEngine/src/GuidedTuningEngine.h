// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.h
// MODIFIED FILE

#ifndef GUIDED_TUNING_ENGINE_H
#define GUIDED_TUNING_ENGINE_H

#include "FilterManager.h"
#include "AdcManager.h"
#include "ui/screens/AutoTuningScreen.h"
#include "pBiosContext.h" // For the context "float"
#include <arduinoFFT.h>

#define GT_SAMPLE_COUNT 512

/**
 * @class GuidedTuningEngine
 * @brief --- DEFINITIVE REFACTOR: The engine is now a collection of non-blocking, discrete functions. ---
 * The monolithic `proposeSettings` function has been replaced by a suite of smaller
 * methods that are called sequentially by the pBiosDataTask's new state machine.
 * This resolves the watchdog timer crash (Heisenbug) and enables a true, multi-stage
 * guided tuning wizard.
 */
class GuidedTuningEngine {
public:
    GuidedTuningEngine();
    ~GuidedTuningEngine();

    // --- Wizard Stage Functions ---
    bool captureSignal(PBiosContext& context, AdcManager& adcManager, AutoTuningScreen& progressScreen);
    bool characterizeSignal(PBiosContext& context, AutoTuningScreen& progressScreen);
    bool optimizeHfStage(PBiosContext& context, AutoTuningScreen& progressScreen);
    bool optimizeLfStage(PBiosContext& context, AutoTuningScreen& progressScreen);
    void applyResults(PBiosContext& context);

private:
    // FFT-related members
    arduinoFFT* _FFT; 
    double* _fftReal;
    double* _fftImag;
};

#endif // GUIDED_TUNING_ENGINE_H