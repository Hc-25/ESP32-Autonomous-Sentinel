# ESP32 Autonomous Sentinel 🛡️🌱

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.4-blue)](https://github.com/espressif/esp-idf)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32-S3](https://img.shields.io/badge/Hardware-ESP32--S3-green)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Status](https://img.shields.io/badge/Status-Active%20Development-brightgreen)](https://github.com/Hc-25/ESP32-Autonomous-Sentinel)
[![Security](https://img.shields.io/badge/Security%20Module-Implemented-success)](https://github.com/Hc-25/ESP32-Autonomous-Sentinel)
[![Agriculture](https://img.shields.io/badge/Agriculture%20Module-Planned-yellow)](https://github.com/Hc-25/ESP32-Autonomous-Sentinel)

An intelligent, multi-purpose IoT system for ESP32-S3 designed to combine security monitoring with smart agriculture capabilities. **Currently implements** on-device AI object detection and PIR-based intrusion alerts with Telegram integration. **Planned features** include adaptive irrigation control and AI-powered crop monitoring.

> **⚠️ Current Status:** Security & monitoring module is **fully implemented and tested**. Smart agriculture features are **under development**.

## 🌟 Features

### ✅ Implemented Features (Current Version)

#### 🔒 Security & Monitoring
- ✅ **PIR Motion Detection**: Wake from deep sleep on motion (~10-20mA in armed state)
- ✅ **On-Device AI**: ESP-DL powered object detection (no cloud required)
- ✅ **Intrusion Detection**: Real-time alerts with photo capture
- ✅ **Smart Notifications**: Telegram bot integration with photo alerts
- ✅ **Ultra-Low Power**: ~5-10µA during cooldown periods
- ✅ **Cooldown System**: Prevents notification spam (configurable)

#### ⚙️ System Infrastructure
- ✅ **Modular Architecture**: Clean separation of concerns for easy customization
- ✅ **SD Card Storage**: AI model loading from SD card
- ✅ **WiFi Management**: Automatic connection and reconnection
- ✅ **Power Management**: Deep sleep states with multiple wake sources
- ✅ **Error Handling**: Comprehensive error checking and recovery

### 🚧 Planned Features (Under Development)

#### 🌾 Smart Agriculture Module
- 🚧 **Adaptive Irrigation**: Intelligent watering based on soil moisture, weather, and crop needs
- 🚧 **Crop Progress Monitoring**: AI-powered image segmentation for growth tracking
- 🚧 **Environmental Sensing**: Temperature, humidity, and soil moisture monitoring
- 🚧 **Automated Scheduling**: Time-based and sensor-triggered irrigation control
- 🚧 **Weather Integration**: API-based irrigation scheduling

#### 🔧 Advanced System Features
- 🚧 **OTA Updates**: Over-the-air firmware updates via WiFi
- 🚧 **Remote Commands**: Telegram bot commands for system control
- 🚧 **Web Dashboard**: Configuration and monitoring interface
- 🚧 **Data Logging**: Historical data storage and analysis
- 🚧 **Multi-zone Control**: Different detection/irrigation zones

## 🎯 Use Cases

### ✅ Current Applications (Implemented)
- ✅ Home security monitoring
- ✅ Wildlife detection cameras
- ✅ Package delivery notifications
- ✅ Intrusion detection systems
- ✅ Perimeter monitoring with alerts

### 🚧 Future Applications (Planned)
- 🚧 Smart greenhouse automation
- 🚧 Precision irrigation systems
- 🚧 Crop health monitoring
- 🚧 Growth stage detection
- 🚧 Automated watering schedules
- 🚧 Protected agriculture with integrated security

## 🔧 Hardware Requirements

### ✅ Current Implementation (Required)
- **ESP32-S3** (N16R8 or similar with PSRAM)
- **OV2640 Camera** module
- **PIR Motion Sensor** (HC-SR501 or similar)
- **MicroSD Card** (for AI models, min 4GB)
- **Power Supply** (5V USB or battery)

### 🚧 Future Hardware (For Planned Features)
- **Soil Moisture Sensor** (Capacitive or resistive)
- **DHT22/DHT11** (Temperature & humidity sensor)
- **Relay Module** (for valve control, 5V)
- **Solenoid Valve** (12V DC for water control)
- **Water Pump** (optional, for automated systems)

### Tested Boards
- Freenove ESP32-S3-WROOM N16R8 Camera Board
- ESP32-S3-EYE development board

## 📦 Software Requirements

- ESP-IDF v5.4.x or later
- Python 3.8+ (for IDF tools)
- ESP-DL library (automatically managed)

## 🚀 Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/Hc-25/ESP32-Autonomous-Sentinel.git
cd ESP32-Autonomous-Sentinel
```

### 2. Set ESP-IDF Environment

```bash
. $HOME/esp/esp-idf/export.sh
```

### 3. Configure the Project

```bash
idf.py menuconfig
```

Key configurations:
- **Component config → ESP-DL → Model Settings**: Select model location (SD card recommended)
- **Component config → Camera**: Adjust camera settings for your hardware
- **main → WiFi Configuration**: Set your WiFi credentials
- **main → Telegram Configuration**: Set bot token and chat ID

### 4. Configure Credentials

Edit `main/config/credentials.hpp`:

```cpp
namespace credentials {
    constexpr const char* WIFI_SSID = "YourWiFiSSID";
    constexpr const char* WIFI_PASSWORD = "YourPassword";
    constexpr const char* TELEGRAM_BOT_TOKEN = "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11";
    constexpr const char* TELEGRAM_CHAT_ID = "your_chat_id";
}
```

### 5. Prepare SD Card

Format SD card as FAT32 and create directory structure:

```
/sdcard
  └── models
      └── s3  (or p4 for ESP32-P4)
          └── (model files copied during build)
```

### 6. Build and Flash

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## 📁 Project Structure

```
ESP32-Autonomous-Sentinel/
├── components/
│   └── detect/                    # Generic detection model component
│       ├── detect.hpp             # Detection API
│       ├── detect.cpp             # Implementation
│       ├── models/                # Pre-trained models
│       │   ├── s3/
│       │   └── p4/
│       ├── CMakeLists.txt
│       ├── Kconfig
│       └── idf_component.yml
├── main/
│   ├── app_main.cpp               # Application entry point
│   ├── config/                    # Configuration headers
│   │   ├── board_config.hpp       # Pin definitions
│   │   ├── app_config.hpp         # Application settings
│   │   └── credentials.hpp        # WiFi/Telegram credentials
│   ├── drivers/                   # Hardware drivers
│   │   ├── camera_driver.hpp/cpp
│   │   └── sdcard_driver.hpp/cpp
│   ├── network/                   # Network modules
│   │   ├── wifi_manager.hpp/cpp
│   │   └── telegram_client.hpp/cpp
│   ├── power/                     # Power management
│   │   └── sleep_manager.hpp/cpp
│   ├── detection/                 # Detection wrapper
│   │   └── detector.hpp/cpp
│   ├── CMakeLists.txt
│   └── idf_component.yml
├── CMakeLists.txt                 # Root build config
├── partitions.csv                 # Flash partition table
├── sdkconfig.defaults             # Default config
├── README.md
└── LICENSE
```

## 🏗️ Architecture

The system follows a **layered modular architecture**:

```
┌─────────────────────────────────┐
│     Application Layer           │  app_main.cpp - State machine
├─────────────────────────────────┤
│     Detection Layer             │  detector - AI wrapper
├─────────────────────────────────┤
│     Network Layer               │  WiFi, Telegram client
├─────────────────────────────────┤
│     Driver Layer                │  Camera, SD card
├─────────────────────────────────┤
│     Power Management            │  Deep sleep control
├─────────────────────────────────┤
│     Configuration Layer         │  Pins, settings, credentials
└─────────────────────────────────┘
```

## 🔄 State Machine

```
POWER_ON → PIR_WARMUP → DEEP_SLEEP (armed)
    ↓
PIR_TRIGGER → MOUNT_SD → INIT_CAMERA → CAPTURE
    ↓
AI_DETECT → [DETECTED?]
    ↓
WIFI_CONNECT → TELEGRAM_SEND → COOLDOWN → DEEP_SLEEP
    ↓
TIMER_WAKEUP → DEEP_SLEEP (re-arm)
```

## ⚙️ Configuration Options

### Detection Settings (`app_config.hpp`)

```cpp
namespace config::detection {
    constexpr float MIN_CONFIDENCE = 0.70f;  // Minimum detection confidence
}
```

### Cooldown Settings

```cpp
namespace config::cooldown {
    constexpr uint64_t PERIOD_SEC = 3600;  // 1 hour cooldown
}
```

### PIR Settings

```cpp
namespace config::pir {
    constexpr uint32_t WARMUP_MS = 60000;  // 60 second warmup
}
```

## �️ Development Roadmap

### ✅ Phase 1: Security System (Completed)
- [x] PIR motion detection with deep sleep
- [x] Camera capture and image processing
- [x] ESP-DL AI model integration
- [x] Telegram bot notifications
- [x] Power management and cooldown system
- [x] SD card model loading
- [x] WiFi connection management

### 🚧 Phase 2: Smart Agriculture (In Progress)
- [ ] Soil moisture sensor integration
- [ ] Environmental sensors (DHT22)
- [ ] Irrigation control system
- [ ] Adaptive watering algorithms
- [ ] Weather API integration
- [ ] Crop image segmentation model

### 📅 Phase 3: Advanced Features (Planned)
- [ ] OTA firmware updates
- [ ] Telegram command interface
- [ ] Web dashboard for configuration
- [ ] Multi-zone management
- [ ] Historical data logging and analytics
- [ ] Mobile app integration

## 📊 Performance Metrics (Current Implementation)

| Metric | Value |
|--------|-------|
| Deep Sleep (Armed) | ~10-20mA |
| Deep Sleep (Cooldown) | ~5-10µA |
| Active Detection | ~200-300mA |
| Detection Time | ~5-10 seconds |
| Wake-to-Notify | ~15-20 seconds |

## 🔐 Security Considerations

- **Credentials**: Never commit `credentials.hpp` to version control
- **Telegram Bot**: Use a dedicated bot token, don't share
- **WiFi**: Consider WPA3 for enhanced security
- **SD Card**: Encrypt sensitive data if needed

## 🐛 Troubleshooting

### Camera Init Failed
- Check camera pin definitions in `board_config.hpp`
- Verify camera power supply (3.3V)
- Ensure PSRAM is enabled in menuconfig

### SD Card Mount Failed
- Format SD card as FAT32
- Check SD card pin connections
- Verify SD card is not write-protected

### Detection Not Working
- Ensure model files are on SD card in correct path
- Check PSRAM is enabled (required for models)
- Verify sufficient flash partition size

### Telegram Not Sending
- Verify WiFi credentials are correct
- Check bot token and chat ID
- Ensure firewall allows HTTPS (port 443)

## 🛠️ Customization

### Current Customization Options

#### Detection Model
The system uses ESP-DL's Pico detection model. To adjust:

1. Change confidence threshold in `main/config/app_config.hpp`
2. Replace model files in `components/detect/models/`
3. Adjust detection parameters in `components/detect/detect.hpp`

#### Power Management
Modify sleep and cooldown parameters in `main/config/app_config.hpp`:
- Cooldown period duration
- PIR warmup time
- WiFi connection timeout

#### Hardware Configuration
Update GPIO pins in `main/config/board_config.hpp` for different boards.

### Adding New Features

The modular architecture makes it easy to extend:
- **New Sensors**: Add drivers in `main/drivers/`
- **Additional Networks**: Extend `main/network/` with new clients
- **Advanced Detection**: Modify detection wrapper in `main/detection/`

**Contributions welcome for agriculture module development!**

## 📚 Dependencies

- [ESP-IDF](https://github.com/espressif/esp-idf) v5.4+
- [ESP-DL](https://github.com/espressif/esp-dl) v3.2+ (auto-managed)
- [ESP32-Camera](https://github.com/espressif/esp32-camera) v2.0+ (auto-managed)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome, especially for the **smart agriculture module**! 

### Priority Areas for Contribution
- 🌾 Soil moisture sensor integration
- 🌡️ Environmental sensor drivers (DHT22, BME280)
- 💧 Irrigation control logic
- 🌤️ Weather API integration
- 🌱 Crop segmentation models
- 🔄 OTA update implementation
- 📊 Web dashboard development

Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 👨‍💻 Author

**Hemanth Chigurupati**
- GitHub: [@Hc-25](https://github.com/Hc-25)

## 🙏 Acknowledgments

- Espressif for ESP-IDF and ESP-DL frameworks
- ESP32 community for hardware support
- Contributors and testers

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Hc-25/ESP32-Autonomous-Sentinel/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Hc-25/ESP32-Autonomous-Sentinel/discussions)

---

**⭐ Star this repo if you find it useful!**
