/**
 * @file detector.cpp
 * @brief Object detector implementation
 */

#include "detector.hpp"
#include "app_config.hpp"

#include "esp_log.h"
#include "esp_heap_caps.h"

// ESP-DL headers
#include "dl_image_jpeg.hpp"
#include "detect.hpp"

#include <memory>

static const char* TAG = "Detector";

namespace detection {

Detector::Detector()
    : m_model(nullptr)
{
}

Detector::~Detector() {
    // Model cleanup is handled by unique_ptr in detect()
}

DetectionResult Detector::detect(camera_fb_t* frame) {
    DetectionResult result = {
        .detected = false,
        .confidence = 0.0f,
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0
    };
    
    if (!frame || !frame->buf || frame->len == 0) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return result;
    }
    
    ESP_LOGI(TAG, "Processing frame: %zu bytes, %dx%d",
             frame->len, frame->width, frame->height);
    
    // Decode JPEG to RGB888
    dl::image::jpeg_img_t jpegImg = {
        .data = frame->buf,
        .data_len = frame->len
    };
    
    auto img = dl::image::sw_decode_jpeg(jpegImg, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    
    if (!img.data) {
        ESP_LOGE(TAG, "JPEG decode failed");
        return result;
    }
    
    ESP_LOGI(TAG, "Decoded to RGB888: %dx%d, %zu bytes",
             img.width, img.height, 
             static_cast<size_t>(img.width * img.height * 3));
    
    // Create detector and run inference
    // Using unique_ptr for automatic cleanup
    auto detector = std::make_unique<Detect>();
    
    auto& detections = detector->run(img);
    
    // Process results
    if (!detections.empty()) {
        // Get the first (highest confidence) detection
        const auto& det = detections.front();
        
        result.detected = true;
        result.confidence = det.score;
        
        // Extract bounding box if available
        if (det.box.size() >= 4) {
            result.x = static_cast<int>(det.box[0]);
            result.y = static_cast<int>(det.box[1]);
            result.width = static_cast<int>(det.box[2] - det.box[0]);
            result.height = static_cast<int>(det.box[3] - det.box[1]);
        }
        
        ESP_LOGI(TAG, "✓ OBJECT DETECTED! Confidence: %.3f", result.confidence);
        ESP_LOGI(TAG, "  Bounding box: (%d, %d) %dx%d",
                 result.x, result.y, result.width, result.height);
    } else {
        ESP_LOGI(TAG, "No object detected");
    }
    
    // Clean up decoded image buffer
    // Note: dl::image allocates using heap_caps_malloc with SPIRAM
    if (img.data) {
        heap_caps_free(img.data);
    }
    
    return result;
}

} // namespace detection
