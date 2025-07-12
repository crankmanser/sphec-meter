// src/presentation/resources/icons.h
// MODIFIED FILE
#pragma once

#include <cstdint>
#include "presentation/common/UI_types.h" // <<< FIX: Include UI_types.h to get the Icon_ID enum

// This file defines the master list of all UI icons and contains their
// raw bitmap data, stored in PROGMEM to save RAM.
// NOTE: The bitmap data here is placeholder data for visual testing.
// It should be replaced with real 1-bit icon graphics.

// Placeholder 16x16 icon bitmap (a simple square)
const uint8_t placeholder_icon_16x16[] PROGMEM = {
    0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
    0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
    0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xFF, 0xFF
};

// Master array of all icon bitmaps.
const uint8_t* const icon_bitmaps[] = {
    // System Tray
    placeholder_icon_16x16, // ICON_PH_PROBE_NORMAL
    placeholder_icon_16x16, // ICON_PH_PROBE_ATTENTION
    placeholder_icon_16x16, // ICON_EC_PROBE_NORMAL
    placeholder_icon_16x16, // ICON_EC_PROBE_ATTENTION
    placeholder_icon_16x16, // ICON_KPI_100
    placeholder_icon_16x16, // ICON_KPI_75
    placeholder_icon_16x16, // ICON_KPI_50
    placeholder_icon_16x16, // ICON_KPI_25
    placeholder_icon_16x16, // ICON_KPI_0
    placeholder_icon_16x16, // ICON_SD_100
    placeholder_icon_16x16, // ICON_SD_75
    placeholder_icon_16x16, // ICON_SD_50
    placeholder_icon_16x16, // ICON_SD_25
    placeholder_icon_16x16, // ICON_SD_0
    placeholder_icon_16x16, // ICON_SD_ERROR
    placeholder_icon_16x16, // ICON_SD_ACTIVITY
    placeholder_icon_16x16, // ICON_BUS_3V3_OK
    placeholder_icon_16x16, // ICON_BUS_3V3_ATTENTION
    placeholder_icon_16x16, // ICON_BUS_5V_OK
    placeholder_icon_16x16, // ICON_BUS_5V_ATTENTION
    placeholder_icon_16x16, // ICON_WIFI_CONNECTED
    placeholder_icon_16x16, // ICON_WIFI_CONNECTING
    placeholder_icon_16x16, // ICON_WIFI_AP_MODE
    placeholder_icon_16x16, // ICON_WIFI_DISCONNECTED
    placeholder_icon_16x16, // ICON_WIFI_ACTIVITY
    // State Stack
    placeholder_icon_16x16, // ICON_STATE_CRITICAL_ERROR
    placeholder_icon_16x16, // ICON_STATE_BATTERY_CRITICAL
    placeholder_icon_16x16, // ICON_STATE_CALIBRATING
    placeholder_icon_16x16, // ICON_STATE_LOGGING_ACTIVE
    placeholder_icon_16x16, // ICON_STATE_WIFI_CONNECTING
    placeholder_icon_16x16, // ICON_STATE_NOTIFICATION
    placeholder_icon_16x16  // ICON_STATE_IDLE
};