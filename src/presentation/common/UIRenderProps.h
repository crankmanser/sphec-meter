// src/presentation/common/UIRenderProps.h
// MODIFIED FILE
#pragma once

#include "UI_types.h"
#include "presentation/blocks/MenuBlock.h" // <<< ADDED: Include the new block props
#include <vector>

// --- Status Area Props ---
struct TopStatusProps {
    std::string date_text;
    std::string time_text;
    Icon_ID ph_probe_icon = Icon_ID::ICON_PH_PROBE_NORMAL;
    Icon_ID ph_kpi_icon = Icon_ID::ICON_KPI_100;
    Icon_ID ec_probe_icon = Icon_ID::ICON_EC_PROBE_NORMAL;
    Icon_ID ec_kpi_icon = Icon_ID::ICON_KPI_100;
    Icon_ID sd_card_icon = Icon_ID::ICON_SD_100;
    Icon_ID wifi_icon = Icon_ID::ICON_WIFI_DISCONNECTED;
    Icon_ID bus_3v3_icon = Icon_ID::ICON_BUS_3V3_OK;
    Icon_ID bus_5v_icon = Icon_ID::ICON_BUS_5V_OK;
};

struct StateStackProps {
    Icon_ID icon1 = Icon_ID::ICON_STATE_IDLE;
    Icon_ID icon2 = Icon_ID::ICON_STATE_IDLE;
    Icon_ID icon3 = Icon_ID::ICON_STATE_IDLE;
};

// --- Main Screen Content Props (Now includes blocks) ---
struct OledProps {
    bool is_dirty = true;
    // Generic text lines
    std::string line1;
    std::string line2;
    std::string line3;
    // Block-specific props
    MenuBlockProps menu_props; // <<< ADDED: Props for the MenuBlock
};

// --- Master Render Props ---
struct UIRenderProps {
    bool show_top_bar = true;
    TopStatusProps top_status_props;
    StateStackProps state_stack_props;
    OledProps oled_top_props;
    OledProps oled_middle_props;
    OledProps oled_bottom_props;
    ButtonPrompt button_prompts;
};