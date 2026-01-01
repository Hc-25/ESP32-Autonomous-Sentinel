/**
 * @file wifi_manager.cpp
 * @brief WiFi Manager implementation
 */

#include "wifi_manager.hpp"
#include "credentials.hpp"
#include "app_config.hpp"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include <cstring>

static const char* TAG = "WiFiManager";

namespace network {

WifiManager::WifiManager()
    : m_eventGroup(nullptr)
    , m_wifiEventHandle(nullptr)
    , m_ipEventHandle(nullptr)
    , m_initialized(false)
{
}

WifiManager::~WifiManager() {
    disconnect();
}

esp_err_t WifiManager::initNvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs erase, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

void WifiManager::eventHandler(void* arg, esp_event_base_t eventBase,
                               int32_t eventId, void* eventData) {
    WifiManager* self = static_cast<WifiManager*>(arg);
    
    if (eventBase == WIFI_EVENT) {
        switch (eventId) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started, connecting...");
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* event = 
                    static_cast<wifi_event_sta_disconnected_t*>(eventData);
                ESP_LOGW(TAG, "Disconnected, reason: %d", event->reason);
                
                // Attempt reconnection
                esp_wifi_connect();
                break;
            }
            
            default:
                break;
        }
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        
        if (self->m_eventGroup) {
            xEventGroupSetBits(self->m_eventGroup, CONNECTED_BIT);
        }
    }
}

esp_err_t WifiManager::connect() {
    esp_err_t ret;
    
    // Initialize NVS
    ret = initNvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create event group
    m_eventGroup = xEventGroupCreate();
    if (!m_eventGroup) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize network interface
    ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Netif init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create default event loop
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Event loop create failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create default WiFi station
    esp_netif_create_default_wifi_sta();
    
    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register event handlers
    ret = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID,
        &WifiManager::eventHandler, this,
        &m_wifiEventHandle
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi event handler register failed");
        return ret;
    }
    
    ret = esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &WifiManager::eventHandler, this,
        &m_ipEventHandle
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "IP event handler register failed");
        return ret;
    }
    
    // Configure WiFi
    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.ssid), 
                 credentials::wifi::SSID, 
                 sizeof(wifiConfig.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.password), 
                 credentials::wifi::PASSWORD, 
                 sizeof(wifiConfig.sta.password) - 1);
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) return ret;
    
    ret = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (ret != ESP_OK) return ret;
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) return ret;
    
    m_initialized = true;
    
    ESP_LOGI(TAG, "Waiting for connection (timeout: %d ms)...", 
             config::timing::WIFI_TIMEOUT_MS);
    
    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(
        m_eventGroup,
        CONNECTED_BIT | FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(config::timing::WIFI_TIMEOUT_MS)
    );
    
    if (bits & CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to %s", credentials::wifi::SSID);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

void WifiManager::disconnect() {
    if (!m_initialized) {
        return;
    }
    
    ESP_LOGI(TAG, "Disconnecting WiFi...");
    
    // Unregister event handlers
    if (m_wifiEventHandle) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                               m_wifiEventHandle);
        m_wifiEventHandle = nullptr;
    }
    
    if (m_ipEventHandle) {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                               m_ipEventHandle);
        m_ipEventHandle = nullptr;
    }
    
    // Stop and deinit WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    
    // Delete event group
    if (m_eventGroup) {
        vEventGroupDelete(m_eventGroup);
        m_eventGroup = nullptr;
    }
    
    m_initialized = false;
    ESP_LOGI(TAG, "WiFi disconnected and cleaned up");
}

bool WifiManager::isConnected() const {
    if (!m_eventGroup) {
        return false;
    }
    
    EventBits_t bits = xEventGroupGetBits(m_eventGroup);
    return (bits & CONNECTED_BIT) != 0;
}

} // namespace network
