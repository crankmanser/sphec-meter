// src/app/AppContext.h
// MODIFIED FILE
#pragma once

/**
 * @file AppContext.h
 * @brief Defines the central context for the entire application.
 */

// Forward-declare all manager and driver classes
class BleManager;
class ButtonManager;
class DisplayManager;
class EncoderManager;
class LDRManager;
class LiquidTempManager;
class AmbientTempManager;
class AmbientHumidityManager;
class MqttManager;
class NoiseAnalysisManager;
class PCF8563_Driver;
class PowerManager;
class RawSensorReader;
class RtcManager;
class SensorProcessor;
class StateManager;
class StorageManager;
class TCA9548_Manual_Driver;
class TelemetrySerializer;
class UIManager;
class WebService;
class WifiManager;
class INA219_Driver; 
class ADS1118_Driver; 
class DS18B20_Driver; 
class DHT_Driver;     
class LDR_Driver;    

#include <Wire.h>

// The main application context struct.
struct AppContext {
    // UI & State Managers
    StateManager* stateManager;
    UIManager* uiManager;
    DisplayManager* displayManager;

    // Pointer to the main I2C bus object ---
    TwoWire* i2c;

    // IO Managers
    ButtonManager* buttonManager;
    EncoderManager* encoderManager;

    // Core System Managers
    StorageManager* storageManager;
    RtcManager* rtcManager;
    PowerManager* powerManager;

    // Sensor Engine Managers
    RawSensorReader* rawSensorReader;
    SensorProcessor* sensorProcessor;
    LiquidTempManager* liquidTempManager;
    AmbientTempManager* ambientTempManager;
    AmbientHumidityManager* ambientHumidityManager;
    LDRManager* ldrManager;
    NoiseAnalysisManager* noiseAnalysisManager;

    // Connectivity & API Managers
    WifiManager* wifiManager;
    MqttManager* mqttManager;
    BleManager* bleManager;
    WebService* webService;
    TelemetrySerializer* telemetrySerializer;

    // Low-level HAL Drivers (if direct access is needed)
    TCA9548_Manual_Driver* tca;
    PCF8563_Driver* rtc_driver;
    INA219_Driver* ina219; 
    ADS1118_Driver* adc1; 
    ADS1118_Driver* adc2; 
    DS18B20_Driver* ds18b20; 
    DHT_Driver* dht; 
    LDR_Driver* ldr; 
};