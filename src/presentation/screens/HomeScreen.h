// src/presentation/screens/HomeScreen.h
// MODIFIED FILE
#pragma once

#include "presentation/screens/Screen.h"
#include "data_models/SensorData_types.h"
#include "freertos/FreeRTOS.h"  
#include "freertos/semphr.h"     


// Forward declare global data
extern ProcessedSensorData g_processed_data;
extern SemaphoreHandle_t g_processed_data_mutex;

class HomeScreen : public Screen {
public:
    void handleInput(const InputEvent& event) override;
    UIRenderProps getRenderProps() override;
};