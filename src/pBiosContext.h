// File Path: /src/pBiosContext.h
// NEW FILE

#ifndef PBIOS_CONTEXT_H
#define PBIOS_CONTEXT_H

#include "FilterManager.h"
#include "AdcManager.h"

// This struct holds the shared state for the pBIOS UI engine, allowing
// different screens to communicate the user's choices.
struct PBiosContext {
    FilterManager* selectedFilter = nullptr;
    uint8_t selectedAdcIndex = 0;
    uint8_t selectedAdcInput = 0;
};

// --- Correctly declare global variables as 'extern' ---
// This tells any file that includes this header that these variables
// exist somewhere else (in main.cpp) and can be used.
extern FilterManager phFilter;
extern FilterManager ecFilter;
extern FilterManager v3_3_Filter;
extern FilterManager v5_0_Filter;

#endif // PBIOS_CONTEXT_H