// src/boot/boot_sequence.h
// NEW FILE
#pragma once

/**
 * @brief The master boot sequence handler for the SpHEC Meter.
 * * This function orchestrates the entire boot process, from initial hardware
 * checks to RTOS startup and final application handoff. It is designed to
 * be the single entry point called from main.cpp after global object
 * instantiation.
 */
void runBootSequence();
