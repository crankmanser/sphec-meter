// File Path: /src/ui/screens/HardwareTestScreen.h
// MODIFIED FILE

#ifndef HARDWARE_TEST_SCREEN_H
#define HARDWARE_TEST_SCREEN_H

#include "ui/StateManager.h"
#include "HardwareTester.h"
#include <vector>

class HardwareTestScreen : public Screen {
public:
    HardwareTestScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    // --- DEFINITIVE FIX: Update signature to match the base class ---
    void onEnter(StateManager* stateManager, int context = 0) override;

    void updateResults(const std::vector<TestResult>& results);
    void setFinalMessage(const std::string& message);

private:
    std::vector<TestResult> _results;
    std::string _final_message;
};

#endif // HARDWARE_TEST_SCREEN_H