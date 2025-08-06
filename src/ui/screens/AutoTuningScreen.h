// File Path: /src/ui/screens/AutoTuningScreen.h
// NEW FILE

#ifndef AUTO_TUNING_SCREEN_H
#define AUTO_TUNING_SCREEN_H

#include "ui/StateManager.h"

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
     * @brief Sets the progress of the analysis operation.
     * @param percent The progress percentage (0-100).
     */
    void setProgress(int percent);

private:
    int _progress_percent;
};

#endif // AUTO_TUNING_SCREEN_H