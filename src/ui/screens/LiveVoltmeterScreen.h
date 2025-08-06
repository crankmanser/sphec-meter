// File Path: /src/ui/screens/LiveVoltmeterScreen.h
// NEW FILE

#ifndef LIVE_VOLTMETER_SCREEN_H
#define LIVE_VOLTMETER_SCREEN_H

#include "ui/StateManager.h"
#include <vector>
#include <string>

/**
 * @class LiveVoltmeterScreen
 * @brief A pBIOS utility to display live, raw ADC voltage readings.
 */
class LiveVoltmeterScreen : public Screen {
public:
    LiveVoltmeterScreen();
    void handleInput(const InputEvent& event) override;
    void getRenderProps(UIRenderProps* props_to_fill) override;
    void onEnter(StateManager* stateManager) override;

    /**
     * @brief Sets the live voltage value to be displayed.
     * @param voltage The raw voltage in millivolts.
     */
    void setLiveVoltage(double voltage);

    uint8_t getSelectedAdcIndex() const;
    uint8_t getSelectedAdcInput() const;
    bool isMeasuring() const;

private:
    enum class VoltmeterState {
        SELECT_SOURCE,
        MEASURING
    };

    void handleSelectSourceInput(const InputEvent& event);
    void handleMeasuringInput(const InputEvent& event);

    VoltmeterState _current_state;
    std::vector<std::string> _source_menu_items;
    int _selected_index;
    double _live_voltage_mv;
};

#endif // LIVE_VOLTMETER_SCREEN_H