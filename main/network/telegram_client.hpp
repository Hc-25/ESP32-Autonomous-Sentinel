#pragma once

/**
 * @file telegram_client.hpp
 * @brief Telegram Bot API client for sending notifications
 * 
 * Provides methods for:
 * - Sending text messages
 * - Sending photos/documents with captions
 * 
 * @note Uses ESP-TLS with certificate bundle for HTTPS
 */

#include "esp_err.h"
#include <cstdint>
#include <cstddef>

namespace network {

/**
 * @brief Telegram Bot client for sending alerts
 */
class TelegramClient {
public:
    TelegramClient() = default;
    ~TelegramClient() = default;
    
    /**
     * @brief Send a document (photo) with caption
     * 
     * @param data      Pointer to image data (JPEG)
     * @param dataLen   Length of image data in bytes
     * @param caption   Caption text for the document
     * @param filename  Filename for the attachment (default: "capture.jpg")
     * 
     * @return esp_err_t 
     *         - ESP_OK on success
     *         - ESP_FAIL on HTTP error
     *         - Other ESP error codes
     */
    esp_err_t sendDocument(const uint8_t* data, size_t dataLen,
                          const char* caption,
                          const char* filename = "capture.jpg");
    
    /**
     * @brief Send a text message
     * 
     * @param message Text message to send
     * 
     * @return esp_err_t 
     */
    esp_err_t sendMessage(const char* message);

private:
    static constexpr const char* BOUNDARY = "X-ESPIDF-MULTIPART-BOUNDARY";
    static constexpr size_t HEADER_BUFFER_SIZE = 512;
    static constexpr size_t TAIL_BUFFER_SIZE = 128;
};

} // namespace network
