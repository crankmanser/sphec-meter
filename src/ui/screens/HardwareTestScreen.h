// File Path: /src/ui/screens/HardwareTestScreen.h
// MODIFIED FILE

#ifndef HARDWARE_TEST_SCREEN_H
#define HARDWARE_TEST_SCREEN_H

#include "ui/StateManager.h"
#include "HardwareTester.h" // Needs the TestResult struct
#include <vector>

/**
 * @class HardwareTestScreen
 * @brief Displays the real-time results of the hardware self-test.
 */
class HardwareTestScreen : public Screen {
public:
    HardwareTestScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    // --- DEFINITIVE FIX: Update signature to match the base class ---
    void onEnter(StateManager* stateManager, int context = 0) override;

    /**
     * @brief Updates the list of test results to be displayed.
     * @param results The vector of test results from the HardwareTester.
     */
    void updateResults(const std::vector<TestResult>& results);
    
    /**
     * @brief Sets the overall test status message (e.g., "All tests passed!").
     * @param message The final message to display.
     */
    void setFinalMessage(const std::string& message);

private:
    std::vector<TestResult> _results;
    std::string _final_message;
};

#endif // HARDWARE_TEST_SCREEN_H