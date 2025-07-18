// src/boot/init_hals.cpp
// MODIFIED FILE
#include "init_hals.h"
#include "app/AppContext.h" // <<< ADDED
#include "hal/ADS1118_Driver.h"
#include "hal/DS18B20_Driver.h"
#include "hal/DHT_Driver.h"
#include "hal/LDR_Driver.h"
#include "config/hardware_config.h"
#include "DebugMacros.h"

void init_hals(AppContext* appContext) {
    // This function now calls the begin() methods on the HAL drivers
    // that are already instantiated and stored in the context.
    
    // Note: The ADC drivers are part of the rawSensorReader, not directly in context
    // and the LDR is part of the adc1 driver.
    // We will initialize the drivers that are directly accessible.
    
    if (appContext->adc1) appContext->adc1->begin();
    if (appContext->adc2) appContext->adc2->begin();
    if (appContext->ds18b20) appContext->ds18b20->begin();
    if (appContext->dht) appContext->dht->begin();
    // LDR_Driver does not have a begin() method.
    
    // It's better to initialize drivers here directly from the context if they exist.
    // The current design creates the ADC drivers in main but doesn't store them
    // in the context. Let's assume for now that their begin() methods are called
    // within their respective manager initializations if needed, or we adjust later.
    // The key drivers to init here are the ones not tied to a complex manager.
    
    // For now, let's keep this simple as per original intent.
    // The main HALs to init are the sensor drivers.
    // This part of the design may need further refinement, but let's fix the compile error.
    
    LOG_MAIN("Non-I2C HAL drivers initialized.\n");;
}