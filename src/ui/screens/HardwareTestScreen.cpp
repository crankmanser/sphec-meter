// File Path: /src/ui/screens/HardwareTestScreen.cpp
// MODIFIED FILE

#include "HardwareTestScreen.h"
#include <string>
#include "ui/UIManager.h"

HardwareTestScreen::HardwareTestScreen() {}

/**
 * @brief --- DEFINITIVE FIX: Update signature to match the base class ---
 */
void HardwareTestScreen::onEnter(StateManager* stateManager, int context) {
    Screen::onEnter(stateManager);
    _results.clear();
    _final_message = "";
}

void HardwareTestScreen::handleInput(const InputEvent& event) {
    if (!_final_message.empty()) {
        if (event.type == InputEventType::BTN_BACK_PRESS || event.type == InputEventType::BTN_DOWN_PRESS) {
            if (_stateManager) _stateManager->changeState(ScreenState::MAINTENANCE_MENU);
        }
    }
}

void HardwareTestScreen::getRenderProps(UIRenderProps* props_to_fill) {
    props_to_fill->oled_top_props.line1 = "pBios > Hardware Self-Test";
    
    auto formatResult = [](const TestResult& result) {
        std::string status_str;
        switch(result.status) {
            case TestStatus::PASS:    status_str = "OK";   break;
            case TestStatus::FAIL:    status_str = "FAIL"; break;
            case TestStatus::RUNNING: status_str = "...";  break;
        }
        return result.testName + ": " + status_str;
    };

    OledProps& mid = props_to_fill->oled_middle_props;
    if (_results.size() > 0) mid.line1 = formatResult(_results[0]);
    if (_results.size() > 1) mid.line2 = formatResult(_results[1]);
    if (_results.size() > 2) mid.line3 = formatResult(_results[2]);
    
    OledProps& bot = props_to_fill->oled_bottom_props;
    if (_results.size() > 3) bot.line1 = formatResult(_results[3]);
    if (_results.size() > 4) bot.line2 = formatResult(_results[4]);
    if (_results.size() > 5) bot.line3 = formatResult(_results[5]);
    
    if (!_final_message.empty()) {
        bot.line3 = _final_message;
        props_to_fill->button_props.back_text = "Done";
        props_to_fill->button_props.down_text = "Done";
    }
}

void HardwareTestScreen::updateResults(const std::vector<TestResult>& results) {
    _results = results;
}

void HardwareTestScreen::setFinalMessage(const std::string& message) {
    _final_message = message;
}