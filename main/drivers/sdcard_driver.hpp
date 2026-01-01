#pragma once

/**
 * @file sdcard_driver.hpp
 * @brief SD Card driver for FAT filesystem access
 * 
 * Supports:
 * - 1-bit SDMMC mode for ESP32-S3
 * - FAT32 filesystem
 * - Proper GPIO cleanup for deep sleep
 */

#include "esp_err.h"
#include "sdmmc_cmd.h"

namespace drivers {

/**
 * @brief SD Card driver class
 */
class SdCardDriver {
public:
    SdCardDriver();
    ~SdCardDriver();
    
    // Disable copy operations
    SdCardDriver(const SdCardDriver&) = delete;
    SdCardDriver& operator=(const SdCardDriver&) = delete;
    
    /**
     * @brief Mount the SD card filesystem
     * 
     * @return esp_err_t 
     *         - ESP_OK on success
     *         - ESP_FAIL if mount failed
     */
    esp_err_t mount();
    
    /**
     * @brief Unmount filesystem and shutdown SD card
     * 
     * Resets GPIO pins for deep sleep compatibility.
     */
    void shutdown();
    
    /**
     * @brief Check if SD card is mounted
     */
    bool isMounted() const { return m_card != nullptr; }
    
    /**
     * @brief Get the mount point path
     */
    const char* getMountPoint() const;

private:
    sdmmc_card_t* m_card;
    
    /**
     * @brief Reset SD card GPIO pins to input mode
     */
    void resetGpioPins();
};

} // namespace drivers
