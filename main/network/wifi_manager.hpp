#pragma once

/**
 * @file wifi_manager.hpp
 * @brief WiFi connection management for ESP32
 * 
 * Provides a clean interface for:
 * - Station mode initialization
 * - Connection with timeout
 * - Graceful cleanup
 * 
 * @note Thread-safe through FreeRTOS event groups
 */

#include "esp_err.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace network {

/**
 * @brief WiFi Manager class for station mode connectivity
 * 
 * Singleton-like usage pattern:
 * @code
 *   WifiManager wifi;
 *   if (wifi.connect() == ESP_OK) {
 *       // Connected, do network operations
 *   }
 *   wifi.disconnect();
 * @endcode
 */
class WifiManager {
public:
    /**
     * @brief Construct a new WiFi Manager
     */
    WifiManager();
    
    /**
     * @brief Destroy the WiFi Manager, ensuring cleanup
     */
    ~WifiManager();
    
    // Disable copy operations (singleton-like behavior)
    WifiManager(const WifiManager&) = delete;
    WifiManager& operator=(const WifiManager&) = delete;
    
    /**
     * @brief Initialize WiFi subsystem and connect to configured AP
     * 
     * @return esp_err_t 
     *         - ESP_OK on successful connection
     *         - ESP_ERR_TIMEOUT if connection timed out
     *         - Other ESP error codes on failure
     */
    esp_err_t connect();
    
    /**
     * @brief Disconnect and cleanup WiFi resources
     */
    void disconnect();
    
    /**
     * @brief Check if currently connected
     * 
     * @return true if connected and has IP address
     * @return false otherwise
     */
    bool isConnected() const;

private:
    EventGroupHandle_t m_eventGroup;
    esp_event_handler_instance_t m_wifiEventHandle;
    esp_event_handler_instance_t m_ipEventHandle;
    bool m_initialized;
    
    static constexpr int CONNECTED_BIT = BIT0;
    static constexpr int FAIL_BIT = BIT1;
    
    /**
     * @brief Static event handler callback
     */
    static void eventHandler(void* arg, esp_event_base_t eventBase,
                            int32_t eventId, void* eventData);
    
    /**
     * @brief Initialize NVS flash if needed
     */
    esp_err_t initNvs();
};

} // namespace network
