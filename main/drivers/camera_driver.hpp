#pragma once

/**
 * @file camera_driver.hpp
 * @brief OV2640 Camera driver wrapper for ESP32-S3
 * 
 * Provides:
 * - Camera initialization with optimized settings
 * - Frame capture with validation
 * - Low-light optimization for object detection
 * - Proper shutdown for deep sleep
 */

#include "esp_err.h"
#include "esp_camera.h"

namespace drivers {

/**
 * @brief Camera driver class for OV2640 sensor
 */
class CameraDriver {
public:
    CameraDriver();
    ~CameraDriver();
    
    // Disable copy operations
    CameraDriver(const CameraDriver&) = delete;
    CameraDriver& operator=(const CameraDriver&) = delete;
    
    /**
     * @brief Initialize camera with optimized settings
     * 
     * @return esp_err_t 
     *         - ESP_OK on success
     *         - ESP_ERR_CAMERA_NOT_DETECTED if sensor not found
     *         - Other errors on initialization failure
     */
    esp_err_t init();
    
    /**
     * @brief Perform camera warmup for exposure stabilization
     * 
     * Captures and discards frames to allow auto-exposure to settle.
     * 
     * @return true if sufficient valid frames captured
     * @return false if warmup failed
     */
    bool warmup();
    
    /**
     * @brief Capture a single frame
     * 
     * @return camera_fb_t* Frame buffer pointer (caller must return via returnFrame())
     *         nullptr on capture failure
     */
    camera_fb_t* capture();
    
    /**
     * @brief Return a frame buffer to the driver
     * 
     * @param fb Frame buffer to return
     */
    void returnFrame(camera_fb_t* fb);
    
    /**
     * @brief Shutdown camera and reset GPIO pins for deep sleep
     */
    void shutdown();
    
    /**
     * @brief Check if camera is initialized
     */
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized;
    
    /**
     * @brief Apply sensor-specific optimizations
     */
    void applySensorSettings(sensor_t* sensor);
    
    /**
     * @brief Reset all camera GPIO pins to input mode
     */
    void resetGpioPins();
};

} // namespace drivers
