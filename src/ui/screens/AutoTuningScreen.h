// File Path: /src/ui/screens/AutoTuningScreen.h
// MODIFIED FILE

#ifndef AUTO_TUNING_SCREEN_H
#define AUTO_TUNING_SCREEN_H

#include "ui/StateManager.h"
#include <string>

class AutoTuningScreen : public Screen {
public:
    AutoTuningScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;

    /**
     * @brief Sets the progress and label of the analysis operation.
     * @param percent The progress percentage (0-100).
     * @param label The text label to display for the current stage.
     */
    void setProgress(int percent, const std::string& label);

private:
    int _progress_percent;
    std::string _progress_label;
};

#endif // AUTO_TUNING_SCREEN_H

