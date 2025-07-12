#pragma once

// This function populates the global raw data struct with simulated sensor
// values, runs all the processing managers, and prints the final
// processed data for verification. The device will halt after one run.
void run_test_and_halt();