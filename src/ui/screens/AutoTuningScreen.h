// File Path: /src/ui/screens/AutoTuningScreen.h
// MODIFIED FILE

#ifndef AUTO_TUNING_SCREEN_H
#define AUTO_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <string> // Include for std::string

/**
 * @class AutoTuningScreen
 * @brief A temporary screen that displays a progress bar while the GuidedTuningEngine runs.
 *
 * This screen provides visual feedback to the user during the automated
 * signal analysis and parameter proposal phase.
 */
class AutoTuningScreen : public Screen {
public:
    AutoTuningScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    /**
     * @brief --- DEFINITIVE FIX: Updates the function signature to accept a label. ---
     * This brings the function's definition in line with its usage in the
     * GuidedTuningEngine, resolving the compiler error.
     * @param percent The progress percentage (0-100).
     * @param label The text label to display for the current stage.
     */
    void setProgress(int percent, const std::string& label);

private:
    int _progress_percent;
    std::string _progress_label;
};

#endif // AUTO_TUNING_SCREEN_H