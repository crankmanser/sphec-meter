// File Path: /src/pBiosContext.h
// MODIFIED FILE

#ifndef PBIOS_CONTEXT_H
#define PBIOS_CONTEXT_H

#include "FilterManager.h"
#include "AdcManager.h"
#include <string>

struct PBiosContext {
    FilterManager* selectedFilter = nullptr;
    std::string selectedFilterName; // To hold "ph_filter", "ec_filter", etc.
    uint8_t selectedAdcIndex = 0;
    uint8_t selectedAdcInput = 0;
};

extern FilterManager phFilter;
extern FilterManager ecFilter;
extern FilterManager v3_3_Filter;
extern FilterManager v5_0_Filter;

#endif // PBIOS_CONTEXT_H