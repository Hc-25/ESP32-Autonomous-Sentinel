#pragma once

/**
 * @file app_config.hpp
 * @brief Application-level configuration parameters
 * 
 * Contains timing, thresholds, and behavioral settings for the
 * autonomous detection security system.
 * 
 * @note For production, consider moving sensitive data to NVS or Kconfig
 */

#include <cstdint>

namespace config {

// =============================================================================
// Timing Configuration
// =============================================================================
namespace timing {
    // Cooldown period after detection (seconds)
    constexpr int64_t COOLDOWN_SECONDS = 3600;  // 1 hour
    
    // WiFi connection timeout (milliseconds)
    constexpr int WIFI_TIMEOUT_MS = 20000;      // 20 seconds
    
    // HTTP request timeout (milliseconds)
    constexpr int HTTP_TIMEOUT_MS = 30000;      // 30 seconds
    
    // Camera warmup frame count
    constexpr int CAMERA_WARMUP_FRAMES = 25;
    
    // Delay between warmup frames (milliseconds)
    constexpr int CAMERA_WARMUP_DELAY_MS = 35;
    
    // Minimum valid warmup frames required
    constexpr int CAMERA_MIN_VALID_FRAMES = 20;
    
} // namespace timing

// =============================================================================
// Camera Configuration
// =============================================================================
namespace camera {
    // JPEG quality (0-63, lower = better quality)
    constexpr int JPEG_QUALITY = 10;
    
    // Frame buffer count (use 3 for better frame selection)
    constexpr int FB_COUNT = 3;
    
    // Minimum valid frame size (bytes)
    constexpr size_t MIN_FRAME_SIZE = 1024;
    
} // namespace camera

// =============================================================================
// Detection Configuration
// =============================================================================
namespace detection {
    // Minimum confidence score for positive detection
    constexpr float MIN_CONFIDENCE = 0.5f;
    
} // namespace detection

// =============================================================================
// Debug Configuration
// =============================================================================
namespace debug {
    // Enable verbose logging
    constexpr bool VERBOSE_LOGGING = true;
    
    // Save debug images to SD card
    constexpr bool SAVE_DEBUG_IMAGES = false;
    
} // namespace debug

} // namespace config
