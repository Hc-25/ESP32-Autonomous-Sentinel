# ESP32 Autonomous Sentinel - Quick Reference

## 🚀 Quick Commands

### Build & Flash
```bash
# Set target (first time only)
idf.py set-target esp32s3

# Build
idf.py build

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor

# Exit monitor: Ctrl+]
```

### Clean Build
```bash
idf.py fullclean
idf.py build
```

### Configuration
```bash
idf.py menuconfig
```

## 📁 Key Files

| File | Purpose |
|------|---------|
| `main/app_main.cpp` | Main application logic |
| `main/config/credentials.hpp` | WiFi & Telegram credentials |
| `main/config/app_config.hpp` | Detection & timing settings |
| `main/config/board_config.hpp` | GPIO pin definitions |
| `components/detect/` | Detection model component |

## ⚙️ Common Configurations

### Change Detection Confidence
Edit `main/config/app_config.hpp`:
```cpp
constexpr float MIN_CONFIDENCE = 0.70f;  // 0.0 to 1.0
```

### Change Cooldown Period
Edit `main/config/app_config.hpp`:
```cpp
constexpr uint64_t PERIOD_SEC = 3600;  // seconds
```

### Change PIR Warmup Time
Edit `main/config/app_config.hpp`:
```cpp
constexpr uint32_t WARMUP_MS = 60000;  // milliseconds
```

### Change PIR Pin
Edit `main/config/board_config.hpp`:
```cpp
constexpr gpio_num_t SENSOR = GPIO_NUM_4;  // Your GPIO
```

## 🐛 Quick Troubleshooting

### Build Errors

**Error: esp-dl not found**
```bash
idf.py reconfigure
```

**Error: credentials.hpp not found**
```bash
cp main/config/credentials.hpp.template main/config/credentials.hpp
# Edit with your credentials
```

### Runtime Issues

**SD Card Mount Failed**
- Format as FAT32
- Check connections
- Try different card

**Camera Init Failed**
- Enable PSRAM: `idf.py menuconfig` → Component config → ESP32S3-Specific → Support for external RAM
- Verify pin definitions

**WiFi Failed**
- Check credentials
- Use 2.4GHz network
- Verify signal strength

**No Detection**
- Check model on SD card
- Increase lighting
- Lower confidence threshold
- Verify PSRAM enabled

## 📊 Monitoring

### Serial Monitor
```bash
idf.py -p /dev/ttyUSB0 monitor
```

### Check Memory
Look for in logs:
```
Free heap: XXXXX bytes
```

### Check Boot Reason
Logs show:
- `POWER_ON` - First boot
- `EXT0` - PIR trigger
- `TIMER` - Cooldown ended

## 🔧 Advanced

### Enable Debug Logging
```bash
idf.py menuconfig
# Component config → Log output → Default log verbosity → Debug
```

### Partition Table

**Standard (models in SD card):**
```bash
# Use partitions.csv (default)
```

**Models in Flash:**
```bash
idf.py menuconfig
# Partition Table → Custom partition CSV file → partitions2.csv
```

### OTA Updates
Enable OTA partition in `partitions.csv` and implement OTA code.

## 📝 Model Files

### Location on SD Card
```
/sdcard/
  └── models/
      └── s3/              # For ESP32-S3
          └── detect_pico_s8_v1.espdl
      └── p4/              # For ESP32-P4
          └── detect_pico_s8_v1.espdl
```

### Build Artifacts
After build, model is packed in:
```
build/espdl_models/detect.espdl
```

## 🎯 Telegram Bot Commands

### Create Bot
1. Message `@BotFather`
2. Send `/newbot`
3. Follow instructions
4. Copy token

### Get Chat ID
1. Message your bot
2. Visit: `https://api.telegram.org/bot<TOKEN>/getUpdates`
3. Find `"chat":{"id": YOUR_ID}`

## 🔋 Power Optimization

### Current Consumption
- Deep Sleep (Armed): ~10-20mA
- Deep Sleep (Cooldown): ~5-10µA
- Active Detection: ~200-300mA

### Reduce Power
- Use external RTC for wake
- Optimize cooldown period
- Disable unused peripherals
- Use lighter models

## 🌐 Network

### WiFi Optimization
```cpp
// In wifi_manager.cpp
wifi_config.sta.listen_interval = 3;  // Adjust for power saving
```

### Telegram Timeout
```cpp
// In telegram_client.cpp
constexpr int TIMEOUT_MS = 30000;  // Adjust timeout
```

## 📦 Component Dependencies

Auto-managed via `idf_component.yml`:
- `espressif/esp-dl` ~3.2.0
- `espressif/esp32-camera` ~2.0.13

## 🔐 Security

### Protect Credentials
```bash
# Add to .gitignore (already included)
main/config/credentials.hpp
```

### Use NVS for Production
Replace hardcoded credentials with NVS storage for production.

---

**Need more help?** See [README.md](README.md) and [SETUP.md](SETUP.md)
