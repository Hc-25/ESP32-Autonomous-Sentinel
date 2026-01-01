/**
 * @file sleep_manager.cpp
 * @brief Sleep manager implementation
 */

#include "sleep_manager.hpp"
#include "board_config.hpp"
#include "app_config.hpp"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "SleepManager";

// RTC memory for persistent cooldown state (survives deep sleep)
RTC_DATA_ATTR static int64_t s_nextPirAllowTime = 0;

namespace power {

SleepManager::SleepManager() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    
    switch (cause) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            m_wakeReason = WakeReason::POWER_ON;
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            m_wakeReason = WakeReason::PIR_TRIGGER;
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            m_wakeReason = WakeReason::TIMER;
            break;
        default:
            m_wakeReason = WakeReason::UNKNOWN;
            break;
    }
}

WakeReason SleepManager::getWakeReason() const {
    return m_wakeReason;
}

int64_t SleepManager::getCurrentTimeSec() const {
    return esp_timer_get_time() / 1000000;
}

bool SleepManager::isInCooldown() const {
    return s_nextPirAllowTime > getCurrentTimeSec();
}

int64_t SleepManager::getCooldownRemaining() const {
    int64_t remaining = s_nextPirAllowTime - getCurrentTimeSec();
    return (remaining > 0) ? remaining : 0;
}

void SleepManager::startCooldown(int64_t seconds) {
    s_nextPirAllowTime = getCurrentTimeSec() + seconds;
    ESP_LOGI(TAG, "Cooldown started: %lld seconds", seconds);
}

[[noreturn]] void SleepManager::enterDeepSleep() {
    int64_t sleepDuration = -1;
    
    if (isInCooldown()) {
        sleepDuration = getCooldownRemaining();
        ESP_LOGW(TAG, "In cooldown. PIR disabled for %lld seconds", sleepDuration);
        
        // Set timer wake-up
        esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);
    } else {
        ESP_LOGI(TAG, "System armed. Enabling PIR wake-up");
        
        // Enable EXT1 wake-up on PIR GPIO
        esp_sleep_enable_ext1_wakeup(
            1ULL << config::pir::PIN,
            ESP_EXT1_WAKEUP_ANY_HIGH
        );
    }
    
    ESP_LOGI(TAG, "Entering deep sleep...");
    
    // Give logs time to flush
    vTaskDelay(pdMS_TO_TICKS(100));
    
    esp_deep_sleep_start();
    
    // Should never reach here
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}

} // namespace power
