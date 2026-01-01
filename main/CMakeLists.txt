# =============================================================================
# ESP32 Autonomous Sentinel - CMake Configuration
# ESP-IDF v5.4.x Compatible
# =============================================================================
#
# This CMakeLists.txt defines the component structure for a modular
# autonomous detection security system. The project is organized into
# logical layers for maintainability and scalability.
#
# Directory Structure:
# -------------------
# main/
#   ├── app_main.cpp              (application entry point)
#   ├── config/                   (configuration headers)
#   ├── drivers/                  (hardware drivers)
#   ├── network/                  (WiFi & Telegram)
#   ├── power/                    (sleep management)
#   └── detection/                (AI detection wrapper)
#
# =============================================================================

# Source directories (recursively include all subdirectories)
set(src_dirs    
    ./
    ./config
    ./drivers
    ./network
    ./power
    ./detection
)

# Include directories (make headers accessible)
set(include_dirs    
    ./
    ./config
    ./drivers
    ./network
    ./power
    ./detection
)

# Public component dependencies (used in headers)
set(component_requires
    detect                         # ESP-DL object detection model
    espressif__esp32-camera        # Camera driver component
)

# Private component dependencies (used only in implementation)
set(priv_requires
    fatfs                          # FAT filesystem
    sdmmc                          # SD/MMC card interface
    nvs_flash                      # Non-volatile storage
    esp_wifi                       # WiFi subsystem
    esp_event                      # Event loop
    esp_http_client                # HTTP client for Telegram
    esp-tls                        # TLS/SSL support
    esp_driver_sdmmc               # SDMMC driver
    log                            # Logging framework
    esp_netif                      # Network interface
    esp_driver_ledc                # LED PWM controller (for camera clock)
    esp_driver_gpio                # GPIO driver
)

# Register the component with ESP-IDF build system
idf_component_register(
    SRC_DIRS ${src_dirs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${component_requires}
    PRIV_REQUIRES ${priv_requires}
)

# =============================================================================
# Build Options & Compiler Flags
# =============================================================================

# Enable C++17 features (required for std::unique_ptr, etc.)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

# Add compile definitions for debugging (optional)
if(CONFIG_LOG_DEFAULT_LEVEL GREATER 3)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        DEBUG_MODE=1
    )
endif()

# =============================================================================
# Post-Build Information
# =============================================================================

# Display modular structure info during build
message(STATUS "")
message(STATUS "╔═══════════════════════════════════════════════════════════╗")
message(STATUS "║  ESP32 Autonomous Sentinel - Modular Build               ║")
message(STATUS "╚═══════════════════════════════════════════════════════════╝")
message(STATUS "Source directories: ${src_dirs}")
message(STATUS "Component requires: ${component_requires}")
message(STATUS "Private requires: ${priv_requires}")
message(STATUS "")
