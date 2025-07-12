#pragma once

/**
 * @file calibration_config.h
 * @brief Defines the physical reference values for standard calibration solutions at 25°C.
 */

// --- pH Buffer Solution Reference Values (@25°C) ---
#define PH_CAL_POINT_LOW     4.01
#define PH_CAL_POINT_MID     6.86
#define PH_CAL_POINT_HIGH    9.18

// --- EC Solution Reference Values (@25°C) ---
#define EC_CAL_POINT_LOW     84.0    // uS/cm
#define EC_CAL_POINT_MID     1413.0  // uS/cm
#define EC_CAL_POINT_HIGH    12880.0 // uS/cm