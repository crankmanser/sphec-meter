// File Path: /lib/SdManager/src/SdFatConfig.h

#ifndef SdFatConfig_h
#define SdFatConfig_h

// --- SdFat Custom Configuration for SpHEC Meter ---

// This configuration file is included by SdFat.h because we defined
// SD_FAT_CONFIG_INCLUDED in platformio.ini. It allows us to customize
// the library's behavior for our specific hardware and needs.

// Set to 1 to use the ESP32's Virtual File System (VFS).
// This allows SdFat to coexist with other filesystems like SPIFFS.
#define SD_FAT_USE_VFS 1

// Define the file type for VFS.
#if SD_FAT_USE_VFS
#define FILE_TYPE_EXFAT 3
#define FILE_TYPE_FAT 3
#endif

// Disable the definitions for FILE_READ and FILE_WRITE within SdFat.
// The standard ESP32 FS.h also defines these, causing a conflict.
// By setting this to 0, we prevent the redefinition errors.
#define BUILTIN_SDCARD 0

// Set the default SPI clock speed for the SD card.
#define SPI_CLOCK SD_SCK_MHZ(25)

#endif // SdFatConfig_h