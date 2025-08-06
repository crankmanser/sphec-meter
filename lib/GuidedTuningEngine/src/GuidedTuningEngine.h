// File Path: /lib/GuidedTuningEngine/src/GuidedTuningEngine.h
// MODIFIED FILE

#ifndef GUIDED_TUNING_ENGINE_H
#define GUIDED_TUNING_ENGINE_H

#include "FilterManager.h"
#include "AdcManager.h"

#define GT_SAMPLE_COUNT 512

class GuidedTuningEngine {
public:
    GuidedTuningEngine();
    bool proposeSettings(FilterManager* targetFilter, AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput);

private:
    bool captureSignal(AdcManager* adcManager, uint8_t adcIndex, uint8_t adcInput);
    void analyzeSignal();
    void deriveHfParameters(PI_Filter* targetHfFilter);

    /**
     * @brief Derives LF parameters based on the performance of the HF filter.
     * @param hfFilter A pointer to the already-tuned HF filter.
     * @param targetLfFilter A pointer to the LF filter to apply new settings to.
     */
    void deriveLfParameters(PI_Filter* hfFilter, PI_Filter* targetLfFilter);

    int runSimulation(const PI_Filter& params, const double* input_data, double& output_std_dev);

    double _rawSamples[GT_SAMPLE_COUNT];
    double _fftReal[GT_SAMPLE_COUNT];
    double _fftImag[GT_SAMPLE_COUNT];
    
    double _rawStdDev;
    double _peakFrequency;
};

#endif // GUIDED_TUNING_ENGINE_H