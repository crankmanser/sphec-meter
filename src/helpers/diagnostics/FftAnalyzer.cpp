// src/helpers/diagnostics/FftAnalyzer.cpp
// MODIFIED FILE
#include "FftAnalyzer.h"
#include <arduinoFFT.h>

FftResult FftAnalyzer::analyze(const std::vector<float>& samples, int sampling_rate_hz) {
    FftResult result;
    const size_t num_samples = samples.size();

    if (num_samples == 0) {
        return result;
    }

    // The library requires arrays of type double.
    double vReal[num_samples];
    double vImag[num_samples];

    for (size_t i = 0; i < num_samples; ++i) {
        vReal[i] = static_cast<double>(samples[i]);
        vImag[i] = 0.0;
    }
    
    ArduinoFFT<double> fft = ArduinoFFT<double>(vReal, vImag, num_samples, static_cast<double>(sampling_rate_hz));

    // <<< FIX: Corrected all method names to use camelCase as per the library's API. >>>
    fft.dcRemoval();
    fft.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft.compute(FFT_FORWARD);
    fft.complexToMagnitude();
    result.dominant_frequency = fft.majorPeak();

    // The FFT results are now in vReal.
    // We only need to analyze the first half of the results due to symmetry.
    size_t num_bands = num_samples / 2;
    result.frequencies.reserve(num_bands);
    result.magnitudes.reserve(num_bands);

    for (size_t i = 1; i < num_bands; ++i) { // Start at 1 to ignore DC offset
        // Calculate the frequency of the current bin.
        double freq = static_cast<double>(i * sampling_rate_hz) / num_samples;
        double magnitude = vReal[i];

        result.frequencies.push_back(freq);
        result.magnitudes.push_back(magnitude);
    }

    result.is_valid = true;
    return result;
}