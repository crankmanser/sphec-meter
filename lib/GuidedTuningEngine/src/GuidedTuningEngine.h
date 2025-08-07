// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.h
// MODIFIED FILE

#ifndef GUIDED_TUNING_ENGINE_H
#define GUIDED_TUNING_ENGINE_H

#include "FilterManager.h"
#include "AdcManager.h"
#include "ui/screens/AutoTuningScreen.h"
#include <arduinoFFT.h>

#define GT_SAMPLE_COUNT 512

class GuidedTuningEngine {
public:
    GuidedTuningEngine();
    ~GuidedTuningEngine();

    bool proposeSettings(FilterManager* targetFilter, AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput, AutoTuningScreen* progressScreen);

private:
    // Core pipeline functions
    bool captureSignal(AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput);
    void analyzeSignal();
    void deriveHfParameters(PI_Filter* targetHfFilter);
    void deriveLfParameters(PI_Filter* hfFilter, PI_Filter* targetLfFilter);
    
    // Member variables for signal data
    double _rawSamples[GT_SAMPLE_COUNT];
    
    // FFT-related members
    arduinoFFT* _FFT; 
    double* _fftReal;
    double* _fftImag;
    
    // Signal characteristics
    double _rawStdDev;
    double _peakFrequency;
};

#endif // GUIDED_TUNING_ENGINE_H