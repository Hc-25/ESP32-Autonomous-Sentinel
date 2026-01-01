#pragma once

/**
 * @file sleep_manager.hpp
 * @brief Deep sleep and wake-up management
 * 
 * Handles:
 * - PIR sensor wake-up configuration
 * - Timer-based wake-up for cooldown
 * - RTC memory for persistent state
 */

#include "esp_sleep.h"
#include <cstdint>

namespace power {

/**
 * @brief Wake-up reason enumeration
 */
enum class WakeReason {
    POWER_ON,           // Initial power on
    PIR_TRIGGER,        // EXT1 wake-up from PIR
    TIMER,              // Timer wake-up (cooldown ended)
    UNKNOWN             // Other/undefined
};

/**
 * @brief Sleep manager class
 */
class SleepManager {
public:
    SleepManager();
    ~SleepManager() = default;
    
    /**
     * @brief Get the reason for wake-up
     */
    WakeReason getWakeReason() const;
    
    /**
     * @brief Check if system is in cooldown period
     * 
     * @return true if cooldown is active
     */
    bool isInCooldown() const;
    
    /**
     * @brief Get remaining cooldown time in seconds
     * 
     * @return Seconds remaining, or 0 if not in cooldown
     */
    int64_t getCooldownRemaining() const;
    
    /**
     * @brief Start cooldown period
     * 
     * @param seconds Duration of cooldown in seconds
     */
    void startCooldown(int64_t seconds);
    
    /**
     * @brief Enter deep sleep with appropriate wake sources
     * 
     * If in cooldown: Sets timer wake-up
     * If not: Sets PIR (EXT1) wake-up
     * 
     * @note This function does not return
     */
    [[noreturn]] void enterDeepSleep();
    
    /**
     * @brief Get current time in seconds since boot
     * 
     * @return Current time in seconds
     */
    int64_t getCurrentTimeSec() const;

private:
    WakeReason m_wakeReason;
};

} // namespace power
