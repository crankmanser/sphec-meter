// src/presentation/common/UI_types.h
// MODIFIED FILE
#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Defines which of the three physical OLED screens is being referenced.
enum class OLED_ID {
    OLED_TOP,
    OLED_MIDDLE,
    OLED_BOTTOM
};

// <<< FIX: This is now the single source of truth for all Icon IDs.
enum class Icon_ID {
    // --- System Tray Icons ---
    ICON_PH_PROBE_NORMAL,
    ICON_PH_PROBE_ATTENTION,
    ICON_EC_PROBE_NORMAL,
    ICON_EC_PROBE_ATTENTION,
    ICON_KPI_100,
    ICON_KPI_75,
    ICON_KPI_50,
    ICON_KPI_25,
    ICON_KPI_0,
    ICON_SD_100,
    ICON_SD_75,
    ICON_SD_50,
    ICON_SD_25,
    ICON_SD_0,
    ICON_SD_ERROR,
    ICON_SD_ACTIVITY,
    ICON_BUS_3V3_OK,
    ICON_BUS_3V3_ATTENTION,
    ICON_BUS_5V_OK,
    ICON_BUS_5V_ATTENTION,
    ICON_WIFI_CONNECTED,
    ICON_WIFI_CONNECTING,
    ICON_WIFI_AP_MODE,
    ICON_WIFI_DISCONNECTED,
    ICON_WIFI_ACTIVITY,

    // --- State Stack Icons ---
    ICON_STATE_CRITICAL_ERROR,
    ICON_STATE_BATTERY_CRITICAL,
    ICON_STATE_CALIBRATING,
    ICON_STATE_LOGGING_ACTIVE,
    ICON_STATE_WIFI_CONNECTING,
    ICON_STATE_NOTIFICATION,
    ICON_STATE_IDLE,

    // --- MUST BE LAST ---
    ICON_COUNT
};


// A structure to hold the text for a button prompt.
struct ButtonPrompt {
    std::string top_button_text;
    std::string middle_button_text;
    std::string bottom_button_text;
};