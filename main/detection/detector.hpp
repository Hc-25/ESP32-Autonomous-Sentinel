#pragma once

/**
 * @file detector.hpp
 * @brief Wrapper for ESP-DL object detection model
 * 
 * Provides simplified interface for:
 * - JPEG decoding
 * - Model inference
 * - Result interpretation
 */

#include "esp_err.h"
#include "esp_camera.h"
#include <vector>

namespace detection {

/**
 * @brief Detection result structure
 */
struct DetectionResult {
    bool detected;      // Was an object detected?
    float confidence;   // Detection confidence score
    int x;              // Bounding box x
    int y;              // Bounding box y
    int width;          // Bounding box width
    int height;         // Bounding box height
};

/**
 * @brief Object detector class
 */
class Detector {
public:
    Detector();
    ~Detector();
    
    // Disable copy operations
    Detector(const Detector&) = delete;
    Detector& operator=(const Detector&) = delete;
    
    /**
     * @brief Run detection on a camera frame
     * 
     * @param frame Camera frame buffer (JPEG format)
     * @return DetectionResult Detection result
     */
    DetectionResult detect(camera_fb_t* frame);

private:
    void* m_model;  // Opaque pointer to model instance
};

} // namespace detection
