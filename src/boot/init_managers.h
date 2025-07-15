// src/boot/init_managers.h
// MODIFIED FILE
#pragma once

// Forward-declare to avoid circular dependencies
class NoiseAnalysisManager;

/**
 * @brief Instantiates and initializes all manager cabinets.
 * @param noiseAnalysisManager A pointer to the noise analysis manager,
 * which will be null if not in DIAGNOSTICS mode.
 */
void init_managers(NoiseAnalysisManager* noiseAnalysisManager);