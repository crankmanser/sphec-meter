// src/app/common/SystemState.h
// NEW FILE
#pragma once

// Defines the boot modes for the entire system.
enum class BootMode {
    NORMAL,
    DIAGNOSTICS
};

// A global variable to hold the detected boot mode.
// It will be defined in main.cpp and declared here as 'extern' for access by other files.
extern BootMode g_boot_mode;