/**
 * @file telegram_client.cpp
 * @brief Telegram Client implementation
 */

#include "telegram_client.hpp"
#include "credentials.hpp"
#include "app_config.hpp"

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include <cstdio>
#include <cstring>

static const char* TAG = "TelegramClient";

namespace network {

esp_err_t TelegramClient::sendDocument(const uint8_t* data, size_t dataLen,
                                        const char* caption,
                                        const char* filename) {
    if (!data || dataLen == 0) {
        ESP_LOGE(TAG, "Invalid data pointer or length");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Build URL
    char url[256];
    std::snprintf(url, sizeof(url), "%s%s/sendDocument",
                  credentials::telegram::API_BASE_URL,
                  credentials::telegram::BOT_TOKEN);
    
    // Configure HTTP client
    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_POST;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = config::timing::HTTP_TIMEOUT_MS;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_ERR_NO_MEM;
    }
    
    // Build multipart form data
    char header[HEADER_BUFFER_SIZE];
    char tail[TAIL_BUFFER_SIZE];
    
    int headerLen = std::snprintf(header, sizeof(header),
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n"
        "%s\r\n"
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"caption\"\r\n\r\n"
        "%s\r\n"
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"document\"; filename=\"%s\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n",
        BOUNDARY, credentials::telegram::CHAT_ID,
        BOUNDARY, caption,
        BOUNDARY, filename
    );
    
    int tailLen = std::snprintf(tail, sizeof(tail), "\r\n--%s--\r\n", BOUNDARY);
    
    // Set content type header
    char contentType[64];
    std::snprintf(contentType, sizeof(contentType), 
                  "multipart/form-data; boundary=%s", BOUNDARY);
    esp_http_client_set_header(client, "Content-Type", contentType);
    
    // Calculate total content length
    int totalLen = headerLen + static_cast<int>(dataLen) + tailLen;
    
    // Open connection and send data
    esp_err_t err = esp_http_client_open(client, totalLen);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }
    
    // Write header
    int written = esp_http_client_write(client, header, headerLen);
    if (written != headerLen) {
        ESP_LOGE(TAG, "Failed to write header");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // Write image data
    written = esp_http_client_write(client, reinterpret_cast<const char*>(data), 
                                    static_cast<int>(dataLen));
    if (written != static_cast<int>(dataLen)) {
        ESP_LOGE(TAG, "Failed to write image data");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // Write tail
    written = esp_http_client_write(client, tail, tailLen);
    if (written != tailLen) {
        ESP_LOGE(TAG, "Failed to write tail");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    
    // Get response
    int contentLength = esp_http_client_fetch_headers(client);
    int statusCode = esp_http_client_get_status_code(client);
    
    ESP_LOGI(TAG, "Response: status=%d, content_length=%d", statusCode, contentLength);
    
    esp_http_client_cleanup(client);
    
    if (statusCode == 200) {
        ESP_LOGI(TAG, "Document sent successfully!");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Telegram API error, status: %d", statusCode);
        return ESP_FAIL;
    }
}

esp_err_t TelegramClient::sendMessage(const char* message) {
    // Build URL
    char url[256];
    std::snprintf(url, sizeof(url), "%s%s/sendMessage",
                  credentials::telegram::API_BASE_URL,
                  credentials::telegram::BOT_TOKEN);
    
    // Build JSON payload
    char payload[512];
    std::snprintf(payload, sizeof(payload),
                  "{\"chat_id\":\"%s\",\"text\":\"%s\"}",
                  credentials::telegram::CHAT_ID, message);
    
    // Configure HTTP client
    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_POST;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = config::timing::HTTP_TIMEOUT_MS;
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return ESP_ERR_NO_MEM;
    }
    
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, std::strlen(payload));
    
    esp_err_t err = esp_http_client_perform(client);
    int statusCode = esp_http_client_get_status_code(client);
    
    esp_http_client_cleanup(client);
    
    if (err == ESP_OK && statusCode == 200) {
        return ESP_OK;
    }
    
    return ESP_FAIL;
}

} // namespace network
