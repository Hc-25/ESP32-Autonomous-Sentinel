/**
 * @file app_main.cpp
 * @brief Main application entry point for ESP32 Autonomous Sentinel System
 * 
 * ============================================================================
 * ESP32 AUTONOMOUS SENTINEL - MULTI-PURPOSE IoT SYSTEM
 * ============================================================================
 * 
 * Hardware Platform: ESP32-S3 Freenove N16R8 Camera Board
 * ESP-IDF Version: v5.4
 * 
 * System Overview:
 * ---------------
 * This embedded IoT system provides:
 * 
 * SECURITY MODULE:
 * - PIR motion sensor triggered wake-up from ultra-low power deep sleep
 * - OV2640 camera capture with low-light optimization (1280x1024 SXGA)
 * - On-device AI object detection using ESP-DL library
 * - Telegram bot integration for instant notifications with photos
 * - Smart cooldown mechanism to prevent notification spam
 * 
 * IRRIGATION MODULE (Planned):
 * - Soil moisture monitoring with capacitive sensors
 * - Adaptive watering schedules based on environmental data
 * - Weather API integration for intelligent irrigation
 * - Relay-controlled solenoid valves for automated water management
 * 
 * CROP MONITORING (Planned):
 * - AI-powered image segmentation for crop health assessment
 * - Growth stage detection and progress tracking
 * - Automated photo capture at scheduled intervals
 * - Data logging to SD card for historical analysis
 * 
 * REMOTE MANAGEMENT:
 * - OTA firmware updates over WiFi
 * - Telegram bot command interface for system control
 * - Configuration via web dashboard (planned)
 * - Real-time status monitoring and alerts
 * 
 * Architecture:
 * ------------
 * The system follows a modular, layered architecture:
 * 
 * 1. Configuration Layer (config/)
 *    - board_config.hpp: Hardware pin definitions
 *    - app_config.hpp: Application parameters
 *    - credentials.hpp: Network credentials (WiFi, Telegram)
 * 
 * 2. Driver Layer (drivers/)
 *    - camera_driver: OV2640 camera interface
 *    - sdcard_driver: SD card FAT filesystem
 * 
 * 3. Network Layer (network/)
 *    - wifi_manager: WiFi STA connection management
 *    - telegram_client: Telegram Bot API client
 * 
 * 4. Power Management (power/)
 *    - sleep_manager: Deep sleep and wake-up control
 * 
 * 5. Detection Layer (detection/)
 *    - detector: ESP-DL AI model wrapper
 * 
 * 6. Application Layer (app_main.cpp)
 *    - Main control flow and state machine
 * 
 * State Machine:
 * -------------
 * POWER_ON -> PIR_WARMUP -> DEEP_SLEEP (armed)
 * 
 * PIR_TRIGGER -> MOUNT_SD -> INIT_CAMERA -> WARMUP -> CAPTURE 
 *             -> AI_DETECT -> [PERSON?] -> WIFI_CONNECT -> TELEGRAM_SEND 
 *             -> COOLDOWN -> DEEP_SLEEP (timer)
 * 
 * TIMER_WAKEUP -> DEEP_SLEEP (re-arm PIR)
 * 
 * Power Consumption:
 * -----------------
 * - Deep Sleep (PIR armed): ~10-20mA (ESP32-S3 + PIR sensor)
 * - Deep Sleep (cooldown): ~5-10µA (ESP32-S3 only, PIR disabled)
 * - Active (detection): ~200-300mA for 5-10 seconds
 * 
 * Planned Features:
 * -----------------
 * - Smart Irrigation: Soil moisture-based adaptive watering
 * - Crop Monitoring: AI-powered image segmentation for growth tracking
 * - Weather Integration: API-based irrigation scheduling
 * - OTA Updates: Remote firmware and model updates
 * - Command Interface: Telegram bot commands for control
 * - Web Dashboard: Configuration and monitoring interface
 * - Multi-zone Detection: Different thresholds per area
 * - Scheduling: Time-based arming/disarming and irrigation
 * - Data Logging: Local storage with timestamp metadata
 * 
 * @author Hemanth Chigurupati
 * @date 2025
 * @version 1.0.0
 */

#include <memory>
#include <cstdio>

// ESP-IDF core
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Configuration
#include "board_config.hpp"
#include "app_config.hpp"
#include "credentials.hpp"

// Drivers
#include "camera_driver.hpp"
#include "sdcard_driver.hpp"

// Network
#include "wifi_manager.hpp"
#include "telegram_client.hpp"

// Power Management
#include "sleep_manager.hpp"

// Detection
#include "detector.hpp"

static const char* TAG = "SENTINEL";

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Handle initial power-on boot
 * 
 * Performs PIR sensor warmup and enters deep sleep to arm the system.
 */
static void handlePowerOnBoot(power::SleepManager& sleepMgr) {
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║    ESP32 AUTONOMOUS SENTINEL - MULTI-PURPOSE SYSTEM       ║");
    ESP_LOGI(TAG, "║  Security + Smart Agriculture + Remote Management         ║");
    ESP_LOGI(TAG, "║  ESP32-S3 | ESP-IDF v5.4 | OV2640 Camera | AI Detection   ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════╝");
    
    ESP_LOGI(TAG, "PIR sensor warmup: %d ms", config::pir::WARMUP_MS);
    vTaskDelay(pdMS_TO_TICKS(config::pir::WARMUP_MS));
    
    ESP_LOGI(TAG, "Warmup complete. System will arm on next wake.");
    sleepMgr.enterDeepSleep();
}

/**
 * @brief Handle timer wake-up (cooldown ended)
 * 
 * Simply re-arms the PIR sensor and goes back to sleep.
 */
static void handleTimerWakeup(power::SleepManager& sleepMgr) {
    ESP_LOGI(TAG, "═══════════════════════════════════════════════════════════");
    ESP_LOGI(TAG, "TIMER WAKEUP - Cooldown period ended");
    ESP_LOGI(TAG, "═══════════════════════════════════════════════════════════");
    
    ESP_LOGI(TAG, "Re-arming PIR sensor...");
    sleepMgr.enterDeepSleep();
}

/**
 * @brief Handle PIR trigger detection workflow
 * 
 * Complete detection pipeline:
 * 1. Mount SD card (for AI model)
 * 2. Initialize camera
 * 3. Warmup camera (exposure stabilization)
 * 4. Capture frame
 * 5. Run AI detection
 * 6. If person detected: send Telegram notification
 * 7. Cleanup and enter deep sleep
 */
static void handlePirTrigger(power::SleepManager& sleepMgr) {
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║                 🚨 PIR MOTION DETECTED! 🚨                 ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════════════════╝");
    
    // Check if we're still in cooldown (double-check)
    if (sleepMgr.isInCooldown()) {
        int64_t remaining = sleepMgr.getCooldownRemaining();
        ESP_LOGW(TAG, "Still in cooldown period (%lld seconds remaining)", remaining);
        ESP_LOGW(TAG, "This trigger will be ignored.");
        sleepMgr.enterDeepSleep();
    }
    
    // ========================================================================
    // STEP 1: Mount SD Card
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 1: Mounting SD Card...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    drivers::SdCardDriver sdCard;
    if (sdCard.mount() != ESP_OK) {
        ESP_LOGE(TAG, "❌ SD Card mount failed!");
        ESP_LOGE(TAG, "Cannot proceed without AI model storage.");
        sdCard.shutdown();
        sleepMgr.enterDeepSleep();
    }
    ESP_LOGI(TAG, "✓ SD Card mounted at %s", sdCard.getMountPoint());
    
    // ========================================================================
    // STEP 2: Initialize Camera
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 2: Initializing Camera...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    drivers::CameraDriver camera;
    if (camera.init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Camera initialization failed!");
        camera.shutdown();
        sdCard.shutdown();
        sleepMgr.enterDeepSleep();
    }
    ESP_LOGI(TAG, "✓ Camera initialized: OV2640, SXGA (1280x1024), JPEG");
    
    // ========================================================================
    // STEP 3: Camera Warmup (Exposure Stabilization)
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 3: Camera Warmup & Exposure Stabilization...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    if (!camera.warmup()) {
        ESP_LOGE(TAG, "❌ Camera warmup failed!");
        ESP_LOGE(TAG, "Insufficient valid frames captured.");
        camera.shutdown();
        sdCard.shutdown();
        sleepMgr.enterDeepSleep();
    }
    ESP_LOGI(TAG, "✓ Camera ready for capture");
    
    // ========================================================================
    // STEP 4: Capture Frame
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 4: Capturing Frame...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    camera_fb_t* frame = camera.capture();
    if (!frame) {
        ESP_LOGE(TAG, "❌ Frame capture failed!");
        camera.shutdown();
        sdCard.shutdown();
        sleepMgr.enterDeepSleep();
    }
    ESP_LOGI(TAG, "✓ Frame captured: %zu bytes, %dx%d", 
             frame->len, frame->width, frame->height);
    
    // ========================================================================
    // STEP 5: AI Object Detection
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 5: Running AI Detection...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    detection::Detector detector;
    detection::DetectionResult result = detector.detect(frame);
    
    // ========================================================================
    // STEP 6: Action Based on Detection Result
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "═════════════════════════════════════════════════════════════");
    
    if (result.detected && result.confidence >= config::detection::MIN_CONFIDENCE) {
        ESP_LOGI(TAG, "✓ OBJECT DETECTED! Confidence: %.2f%%", result.confidence * 100.0f);
        ESP_LOGI(TAG, "═════════════════════════════════════════════════════════════");
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
        ESP_LOGI(TAG, "STEP 6: Sending Telegram Notification...");
        ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
        
        // Connect to WiFi
        network::WifiManager wifi;
        if (wifi.connect() == ESP_OK && wifi.isConnected()) {
            ESP_LOGI(TAG, "✓ WiFi connected");
            
            // Prepare caption with detection details
            char caption[128];
            std::snprintf(caption, sizeof(caption),
                         "⚠️ INTRUDER ALERT!\n"
                         "Confidence: %.1f%%\n"
                         "Time: %lld sec\n"
                         "Location: (%d,%d) %dx%d",
                         result.confidence * 100.0f,
                         sleepMgr.getCurrentTimeSec(),
                         result.x, result.y, result.width, result.height);
            
            // Send notification
            network::TelegramClient telegram;
            if (telegram.sendDocument(frame->buf, frame->len, caption, 
                                     "intruder_detection.jpg") == ESP_OK) {
                ESP_LOGI(TAG, "✓ Telegram notification sent successfully!");
            } else {
                ESP_LOGE(TAG, "❌ Failed to send Telegram notification");
            }
            
            wifi.disconnect();
        } else {
            ESP_LOGE(TAG, "❌ WiFi connection failed - notification not sent");
        }
        
        // Start cooldown period
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "Starting cooldown period: %lld seconds (%.1f hours)",
                 config::timing::COOLDOWN_SECONDS,
                 config::timing::COOLDOWN_SECONDS / 3600.0f);
        sleepMgr.startCooldown(config::timing::COOLDOWN_SECONDS);
        
    } else {
        ESP_LOGI(TAG, "✗ No person detected (confidence: %.2f%%)", 
                 result.confidence * 100.0f);
        ESP_LOGI(TAG, "═════════════════════════════════════════════════════════════");
        ESP_LOGI(TAG, "False alarm - re-arming immediately");
    }
    
    // ========================================================================
    // STEP 7: Cleanup and Sleep
    // ========================================================================
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    ESP_LOGI(TAG, "STEP 7: Cleanup & Power Down...");
    ESP_LOGI(TAG, "─────────────────────────────────────────────────────────────");
    
    camera.returnFrame(frame);
    camera.shutdown();
    sdCard.shutdown();
    
    ESP_LOGI(TAG, "✓ All hardware shut down");
    ESP_LOGI(TAG, "");
    sleepMgr.enterDeepSleep();
}

// ============================================================================
// MAIN APPLICATION ENTRY POINT
// ============================================================================

/**
 * @brief Main application entry point
 * 
 * This function is called after every wake-up (power on, PIR trigger, timer).
 * It determines the wake reason and dispatches to the appropriate handler.
 */
extern "C" void app_main(void) {
    // Initialize sleep manager (determines wake reason)
    power::SleepManager sleepMgr;
    power::WakeReason wakeReason = sleepMgr.getWakeReason();
    
    // Dispatch to appropriate handler based on wake reason
    switch (wakeReason) {
        case power::WakeReason::POWER_ON:
            handlePowerOnBoot(sleepMgr);
            break;
            
        case power::WakeReason::TIMER:
            handleTimerWakeup(sleepMgr);
            break;
            
        case power::WakeReason::PIR_TRIGGER:
            handlePirTrigger(sleepMgr);
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown wake reason - entering sleep");
            sleepMgr.enterDeepSleep();
            break;
    }
    
    // Should never reach here (enterDeepSleep() is [[noreturn]])
    ESP_LOGE(TAG, "ERROR: Reached end of app_main() - this should never happen!");
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
