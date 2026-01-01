/**
 * @file sdcard_driver.cpp
 * @brief SD Card driver implementation
 */

#include "sdcard_driver.hpp"
#include "board_config.hpp"

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"

static const char* TAG = "SdCardDriver";

namespace drivers {

SdCardDriver::SdCardDriver()
    : m_card(nullptr)
{
}

SdCardDriver::~SdCardDriver() {
    if (m_card) {
        shutdown();
    }
}

esp_err_t SdCardDriver::mount() {
    if (m_card) {
        ESP_LOGW(TAG, "SD card already mounted");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Mounting SD card at %s...", config::sdcard::MOUNT_POINT);
    
    // Mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mountConfig = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false
    };
    
    // SDMMC host configuration
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    
    // Slot configuration for 1-bit mode
    sdmmc_slot_config_t slotConfig = SDMMC_SLOT_CONFIG_DEFAULT();
    slotConfig.width = 1;  // 1-bit mode
    slotConfig.clk = static_cast<gpio_num_t>(config::sdcard::PIN_CLK);
    slotConfig.cmd = static_cast<gpio_num_t>(config::sdcard::PIN_CMD);
    slotConfig.d0 = static_cast<gpio_num_t>(config::sdcard::PIN_D0);
    
    // Mount filesystem
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(
        config::sdcard::MOUNT_POINT,
        &host,
        &slotConfig,
        &mountConfig,
        &m_card
    );
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. Check SD card format (FAT32)");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        }
        m_card = nullptr;
        return ret;
    }
    
    // Log card info
    ESP_LOGI(TAG, "SD card mounted successfully");
    ESP_LOGI(TAG, "  Name: %s", m_card->cid.name);
    ESP_LOGI(TAG, "  Speed: %s", (m_card->max_freq_khz < 26000) ? "Default" : "High Speed");
    ESP_LOGI(TAG, "  Size: %lluMB", 
             ((uint64_t)m_card->csd.capacity) * m_card->csd.sector_size / (1024 * 1024));
    
    return ESP_OK;
}

void SdCardDriver::resetGpioPins() {
    ESP_LOGD(TAG, "Resetting SD card GPIO pins...");
    
    for (size_t i = 0; i < config::sdcard::PINS_COUNT; i++) {
        gpio_num_t pin = config::sdcard::PINS[i];
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_pullup_dis(pin);
        gpio_pulldown_dis(pin);
    }
}

void SdCardDriver::shutdown() {
    if (!m_card) {
        return;
    }
    
    ESP_LOGI(TAG, "Unmounting SD card...");
    
    // Unmount filesystem
    esp_vfs_fat_sdcard_unmount(config::sdcard::MOUNT_POINT, m_card);
    m_card = nullptr;
    
    // Reset GPIO for deep sleep
    resetGpioPins();
    
    ESP_LOGI(TAG, "SD card shutdown complete");
}

const char* SdCardDriver::getMountPoint() const {
    return config::sdcard::MOUNT_POINT;
}

} // namespace drivers
