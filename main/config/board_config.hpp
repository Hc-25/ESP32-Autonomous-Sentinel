#pragma once

/**
 * @file board_config.hpp
 * @brief Hardware pin definitions for ESP32-S3 Freenove N16R8 Camera Board
 * 
 * This file contains all GPIO pin mappings for:
 * - OV2640 Camera Module
 * - SD Card Interface (1-bit SDMMC)
 * - PIR Sensor
 * 
 * @note Modify these values if using a different board variant
 */

#include "driver/gpio.h"

namespace config {

// =============================================================================
// Camera Pin Configuration (OV2640 on Freenove ESP32-S3)
// =============================================================================
namespace camera {
    // Power and Reset
    constexpr int PIN_PWDN   = -1;   // Power down pin (-1 = not used)
    constexpr int PIN_RESET  = -1;   // Reset pin (-1 = not used)
    
    // Clock signals
    constexpr int PIN_XCLK   = 15;   // External clock input
    constexpr int PIN_PCLK   = 13;   // Pixel clock output
    
    // Sync signals
    constexpr int PIN_VSYNC  = 6;    // Vertical sync
    constexpr int PIN_HREF   = 7;    // Horizontal reference
    
    // SCCB (I2C-like) interface for sensor control
    constexpr int PIN_SIOD   = 4;    // SCCB Data (SDA)
    constexpr int PIN_SIOC   = 5;    // SCCB Clock (SCL)
    
    // 8-bit parallel data bus (D0-D7)
    constexpr int PIN_D0     = 11;
    constexpr int PIN_D1     = 9;
    constexpr int PIN_D2     = 8;
    constexpr int PIN_D3     = 10;
    constexpr int PIN_D4     = 12;
    constexpr int PIN_D5     = 18;
    constexpr int PIN_D6     = 17;
    constexpr int PIN_D7     = 16;
    
    // Clock frequency (Hz)
    constexpr int XCLK_FREQ_HZ = 10000000;  // 10MHz for stable operation
    
    // Array of all data pins for bulk operations
    constexpr gpio_num_t DATA_PINS[] = {
        static_cast<gpio_num_t>(PIN_D0),
        static_cast<gpio_num_t>(PIN_D1),
        static_cast<gpio_num_t>(PIN_D2),
        static_cast<gpio_num_t>(PIN_D3),
        static_cast<gpio_num_t>(PIN_D4),
        static_cast<gpio_num_t>(PIN_D5),
        static_cast<gpio_num_t>(PIN_D6),
        static_cast<gpio_num_t>(PIN_D7),
        static_cast<gpio_num_t>(PIN_PCLK),
        static_cast<gpio_num_t>(PIN_XCLK),
        static_cast<gpio_num_t>(PIN_HREF),
        static_cast<gpio_num_t>(PIN_VSYNC)
    };
    constexpr size_t DATA_PINS_COUNT = sizeof(DATA_PINS) / sizeof(DATA_PINS[0]);
    
} // namespace camera

// =============================================================================
// SD Card Pin Configuration (1-bit SDMMC Mode)
// =============================================================================
namespace sdcard {
    constexpr int PIN_CLK    = 39;   // SD Clock
    constexpr int PIN_CMD    = 38;   // SD Command
    constexpr int PIN_D0     = 40;   // SD Data 0
    
    // Mount point in VFS
    constexpr const char* MOUNT_POINT = "/sdcard";
    
    // Array of all SD pins for bulk operations
    constexpr gpio_num_t PINS[] = {
        static_cast<gpio_num_t>(PIN_CLK),
        static_cast<gpio_num_t>(PIN_CMD),
        static_cast<gpio_num_t>(PIN_D0)
    };
    constexpr size_t PINS_COUNT = sizeof(PINS) / sizeof(PINS[0]);
    
} // namespace sdcard

// =============================================================================
// PIR Sensor Configuration
// =============================================================================
namespace pir {
    constexpr gpio_num_t PIN = GPIO_NUM_3;   // PIR signal input
    constexpr int WARMUP_MS  = 3000;         // PIR sensor warmup time
    
} // namespace pir

} // namespace config
