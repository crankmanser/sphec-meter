// File Path: /lib/HardwareTester/src/HardwareTester.h
// MODIFIED FILE

#ifndef HARDWARE_TESTER_H
#define HARDWARE_TESTER_H

#include "SdManager.h"
#include "DisplayManager.h"
#include "AdcManager.h"
#include "TempManager.h"
#include <vector>
#include <string>

// Enum to represent the state of a single test
enum class TestStatus {
    RUNNING,
    PASS,
    FAIL
};

// Struct to hold the result of a single hardware test
struct TestResult {
    std::string testName;
    TestStatus status;
};

/**
 * @class HardwareTester
 * @brief A cabinet for running a pBIOS-safe self-test on critical hardware.
 */
class HardwareTester {
public:
    HardwareTester();

    /**
     * @brief Runs a focused sequence of hardware tests relevant to pBIOS operation.
     */
    void runTests(
        SdManager& sdManager,
        DisplayManager& displayManager,
        AdcManager& adcManager,
        TempManager& tempManager,
        std::vector<TestResult>& results
    );

private:
    // Individual test functions
    void testSdCard(SdManager& sdManager, std::vector<TestResult>& results);
    void testOleds(DisplayManager& displayManager, std::vector<TestResult>& results);
    void testAdcs(AdcManager& adcManager, std::vector<TestResult>& results);
    void testTempSensors(TempManager& tempManager, std::vector<TestResult>& results);
};

#endif // HARDWARE_TESTER_H