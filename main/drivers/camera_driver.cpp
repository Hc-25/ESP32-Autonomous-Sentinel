/**
 * @file camera_driver.cpp
 * @brief Camera driver implementation
 */

#include "camera_driver.hpp"
#include "board_config.hpp"
#include "app_config.hpp"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "CameraDriver";

namespace drivers {

CameraDriver::CameraDriver()
    : m_initialized(false)
{
}

CameraDriver::~CameraDriver() {
    if (m_initialized) {
        shutdown();
    }
}

esp_err_t CameraDriver::init() {
    if (m_initialized) {
        ESP_LOGW(TAG, "Camera already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing camera...");
    
    // Configure camera
    camera_config_t config = {};
    
    // LEDC for clock generation
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    
    // Pin configuration from board config
    config.pin_d0 = config::camera::PIN_D0;
    config.pin_d1 = config::camera::PIN_D1;
    config.pin_d2 = config::camera::PIN_D2;
    config.pin_d3 = config::camera::PIN_D3;
    config.pin_d4 = config::camera::PIN_D4;
    config.pin_d5 = config::camera::PIN_D5;
    config.pin_d6 = config::camera::PIN_D6;
    config.pin_d7 = config::camera::PIN_D7;
    config.pin_xclk = config::camera::PIN_XCLK;
    config.pin_pclk = config::camera::PIN_PCLK;
    config.pin_vsync = config::camera::PIN_VSYNC;
    config.pin_href = config::camera::PIN_HREF;
    config.pin_sccb_sda = config::camera::PIN_SIOD;
    config.pin_sccb_scl = config::camera::PIN_SIOC;
    config.pin_pwdn = config::camera::PIN_PWDN;
    config.pin_reset = config::camera::PIN_RESET;
    
    // Clock and format settings
    config.xclk_freq_hz = config::camera::XCLK_FREQ_HZ;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SXGA;  // 1280x1024 for object detection
    config.jpeg_quality = config::camera::JPEG_QUALITY;
    config.fb_count = config::camera::FB_COUNT;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s (0x%x)", esp_err_to_name(err), err);
        return err;
    }
    
    // Apply sensor-specific settings
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor) {
        applySensorSettings(sensor);
    } else {
        ESP_LOGE(TAG, "Failed to get camera sensor!");
        esp_camera_deinit();
        return ESP_FAIL;
    }
    
    m_initialized = true;
    ESP_LOGI(TAG, "Camera initialized successfully");
    
    return ESP_OK;
}

void CameraDriver::applySensorSettings(sensor_t* sensor) {
    ESP_LOGI(TAG, "Sensor PID: 0x%04x", sensor->id.PID);
    
    if (sensor->id.PID == OV2640_PID) {
        ESP_LOGI(TAG, "Applying OV2640 low-light optimizations");
        
        // ===== EXPOSURE & AUTO-EXPOSURE (CRITICAL FOR LOW LIGHT) =====
        sensor->set_exposure_ctrl(sensor, 1);      // Enable AEC
        sensor->set_aec2(sensor, 1);               // Enable AEC2
        sensor->set_ae_level(sensor, 2);           // AE level: +2 for brighter exposure
        
        // ===== GAIN CONTROL =====
        sensor->set_gain_ctrl(sensor, 1);          // Enable AGC
        sensor->set_gainceiling(sensor, GAINCEILING_8X);  // Higher gain for low light
        
        // ===== IMAGE ADJUSTMENTS =====
        sensor->set_brightness(sensor, 2);         // Max brightness
        sensor->set_contrast(sensor, -2);           // Neutral contrast
        sensor->set_saturation(sensor, 0);         // Neutral saturation
        
        // ===== WHITE BALANCE =====
        sensor->set_whitebal(sensor, 1);           // Enable AWB
        sensor->set_awb_gain(sensor, 1);           // Enable AWB gain
        sensor->set_wb_mode(sensor, 0);            // Auto WB mode
        
        // ===== IMAGE ENHANCEMENT =====
        sensor->set_special_effect(sensor, 0);     // No effects
        sensor->set_bpc(sensor, 1);                // Black pixel cancellation
        sensor->set_wpc(sensor, 1);                // White pixel cancellation
        sensor->set_raw_gma(sensor, 1);            // Raw gamma
        sensor->set_lenc(sensor, 1);               // Lens correction
        
        // ===== GEOMETRY =====
        sensor->set_hmirror(sensor, 0);
        sensor->set_vflip(sensor, 0);
        sensor->set_dcw(sensor, 1);                // Downsampling clock
        
        ESP_LOGI(TAG, "OV2640 settings applied:");
        ESP_LOGI(TAG, "  - AEC/AGC enabled with 8X gain ceiling");
        ESP_LOGI(TAG, "  - Brightness: +2, BPC enabled");
        
    } else {
        ESP_LOGW(TAG, "Unknown sensor (PID: 0x%04x), applying generic settings", 
                 sensor->id.PID);
        
        // Generic low-light settings
        sensor->set_exposure_ctrl(sensor, 1);
        sensor->set_gain_ctrl(sensor, 1);
        sensor->set_brightness(sensor, 2);
    }
}

bool CameraDriver::warmup() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return false;
    }
    
    ESP_LOGI(TAG, "Warming up camera (%d frames, %d ms delay)...",
             config::timing::CAMERA_WARMUP_FRAMES,
             config::timing::CAMERA_WARMUP_DELAY_MS);
    
    int validFrames = 0;
    
    for (int i = 0; i < config::timing::CAMERA_WARMUP_FRAMES; i++) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            if (fb->len > config::camera::MIN_FRAME_SIZE) {
                validFrames++;
            }
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(config::timing::CAMERA_WARMUP_DELAY_MS));
    }
    
    float successRate = (static_cast<float>(validFrames) / 
                        config::timing::CAMERA_WARMUP_FRAMES) * 100.0f;
    
    ESP_LOGI(TAG, "Warmup complete: %d/%d valid frames (%.1f%%)",
             validFrames, config::timing::CAMERA_WARMUP_FRAMES, successRate);
    
    return validFrames >= config::timing::CAMERA_MIN_VALID_FRAMES;
}

camera_fb_t* CameraDriver::capture() {
    if (!m_initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return nullptr;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb) {
        ESP_LOGE(TAG, "Frame capture failed - null buffer");
        return nullptr;
    }
    
    if (fb->len < config::camera::MIN_FRAME_SIZE) {
        ESP_LOGE(TAG, "Frame too small: %zu bytes (min: %zu)",
                 fb->len, config::camera::MIN_FRAME_SIZE);
        esp_camera_fb_return(fb);
        return nullptr;
    }
    
    ESP_LOGI(TAG, "Captured frame: %zu bytes, %dx%d",
             fb->len, fb->width, fb->height);
    
    return fb;
}

void CameraDriver::returnFrame(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

void CameraDriver::resetGpioPins() {
    ESP_LOGD(TAG, "Resetting camera GPIO pins...");
    
    for (size_t i = 0; i < config::camera::DATA_PINS_COUNT; i++) {
        gpio_num_t pin = config::camera::DATA_PINS[i];
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_pullup_dis(pin);
        gpio_pulldown_dis(pin);
    }
}

void CameraDriver::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    ESP_LOGW(TAG, "Shutting down camera...");
    
    // Try to put sensor in low-power mode
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor) {
        // OV2640 standby mode
        sensor->set_reg(sensor, 0x09, 0xFF, 0x10);
    }
    
    // Deinitialize camera
    esp_camera_deinit();
    
    // Reset GPIO pins for deep sleep
    resetGpioPins();
    
    m_initialized = false;
    ESP_LOGI(TAG, "Camera shutdown complete");
}

} // namespace drivers
