// File Path: /test/test_filter_manager/test_main.cpp

#include <Arduino.h>
#include <unity.h>
#include <FilterManager.h>
#include <ConfigManager.h>
#include <FaultHandler.h>

// --- Global Test Objects & Mocks ---
FaultHandler testFaultHandler;
ConfigManager testConfigManager;

// --- Test Suite Setup & Teardown ---
void setUp(void) {}
void tearDown(void) {}

// --- TEST CASES ---

/**
 * @brief Test Case 1: FilterManager Initialization
 */
void test_filter_manager_initialization() {
    // ARRANGE
    FilterManager filterManager;
    // ACT
    bool success = filterManager.begin(testFaultHandler, testConfigManager);
    // ASSERT
    TEST_ASSERT_TRUE(success);
}

/**
 * @brief Test Case 2: Two-Stage Pipeline Processing
 */
void test_filter_manager_pipeline() {
    // ARRANGE: Create and initialize a FilterManager instance.
    FilterManager filterManager;
    filterManager.begin(testFaultHandler, testConfigManager);

    PI_Filter* hfFilter = filterManager.getFilter(0);
    PI_Filter* lfFilter = filterManager.getFilter(1);
    TEST_ASSERT_NOT_NULL(hfFilter);
    TEST_ASSERT_NOT_NULL(lfFilter);

    // --- FIX: Isolate the PI filter stage for this test ---
    // The default median filter window (size 5) was rejecting the new value
    // as a spike. By setting the window size to 1, the median filter becomes
    // transparent, allowing us to test the PI smoothing pipeline directly.
    hfFilter->medianWindowSize = 1;
    lfFilter->medianWindowSize = 1;

    // ARRANGE: Configure the filters with simple, predictable behavior.
    hfFilter->lockSmoothing = 0.5;
    hfFilter->settleThreshold = 1000; // Force into LOCKED state

    lfFilter->lockSmoothing = 0.5;
    lfFilter->settleThreshold = 1000; // Force into LOCKED state

    // ARRANGE: Prime the filters by running them 50 times to ensure their
    // internal state converges to a stable 100.0.
    for (int i = 0; i < 50; ++i) {
        filterManager.process(100.0);
    }
    
    // ACT: Process the new value against the stable state.
    double newValue = 200.0;
    double finalValue = filterManager.process(newValue);
    
    // --- Theoretical Calculation ---
    // With priming complete and the median filter bypassed, the math is now correct:
    // 1. HF filter output: (internal_state=100.0 * 0.5) + (newValue=200.0 * 0.5) = 150.0
    // 2. LF filter output: (internal_state=100.0 * 0.5) + (hf_output=150.0 * 0.5) = 125.0
    double expectedValue = 125.0;

    // ASSERT: Check if the final value is close to the theoretical value.
    TEST_ASSERT_FLOAT_WITHIN(0.01, expectedValue, finalValue);
}


// --- TEST RUNNER ---
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_filter_manager_initialization);
    RUN_TEST(test_filter_manager_pipeline);
    UNITY_END();
}

void loop() {}