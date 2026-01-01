# Setup Guide for ESP32 Autonomous Sentinel

Complete step-by-step guide to get your autonomous sentinel up and running.

## 📋 Prerequisites Checklist

- [ ] ESP32-S3 board with camera
- [ ] PIR motion sensor
- [ ] MicroSD card (4GB+, FAT32 formatted)
- [ ] USB cable for programming
- [ ] Computer with Linux/macOS/Windows
- [ ] ESP-IDF v5.4+ installed
- [ ] Telegram account

## 🔧 Step 1: Hardware Setup

### ESP32-S3 Connections

#### Camera (Usually pre-connected on dev boards)
Most camera boards come with integrated cameras. Verify connections in `main/config/board_config.hpp`.

#### PIR Sensor
```
PIR Sensor    →  ESP32-S3
VCC           →  5V
GND           →  GND
OUT           →  GPIO 4 (configurable)
```

#### SD Card
If not built-in, connect via SPI or SDMMC interface (check `board_config.hpp`).

### Power Supply

- Development: USB cable (ensure 5V 2A minimum)
- Production: Battery or stable 5V power source

## 🖥️ Step 2: Software Installation

### Install ESP-IDF

#### Linux/macOS
```bash
# Install prerequisites
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# Clone ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.4
git submodule update --init --recursive

# Install tools
./install.sh esp32s3

# Set up environment (add to ~/.bashrc for permanent)
. $HOME/esp/esp-idf/export.sh
```

#### Windows
Download and run the [ESP-IDF Windows Installer](https://dl.espressif.com/dl/esp-idf/).

## 📦 Step 3: Project Setup

### Clone Repository
```bash
cd ~/projects
git clone https://github.com/Hc-25/ESP32-Autonomous-Sentinel.git
cd ESP32-Autonomous-Sentinel
```

### Configure Credentials

```bash
# Copy template
cp main/config/credentials.hpp.template main/config/credentials.hpp

# Edit with your credentials
nano main/config/credentials.hpp
```

Fill in:
- WiFi SSID and password
- Telegram bot token and chat ID

### Get Telegram Credentials

#### Create Telegram Bot
1. Open Telegram and search for `@BotFather`
2. Send `/newbot`
3. Choose a name and username
4. Copy the bot token (format: `123456789:ABCdefGHI...`)

#### Get Chat ID
1. Send a message to your bot
2. Visit: `https://api.telegram.org/bot<YOUR_BOT_TOKEN>/getUpdates`
3. Find `"chat":{"id": YOUR_CHAT_ID}`
4. Copy the chat ID number

## ⚙️ Step 4: Configuration

### Configure Project
```bash
idf.py set-target esp32s3
idf.py menuconfig
```

#### Key Configuration Options

**Component config → ESP-DL:**
- Navigate to model settings
- Select model location (recommend SD Card for development)

**Component config → Camera:**
- Verify pin configurations match your board
- Set resolution to SXGA (1280x1024)
- Enable JPEG output

**models: detect:**
- Enable `flash detect_pico_s8_v1`
- Select model location (SD Card recommended)
- Set SD card directory: `models/s3`

### Update Board Configuration

If using different pins, edit `main/config/board_config.hpp`:

```cpp
namespace pins {
    namespace pir {
        constexpr gpio_num_t SENSOR = GPIO_NUM_4;  // Your PIR pin
    }
}
```

## 💾 Step 5: Prepare SD Card

### Format SD Card
- Format as FAT32
- Label: `SENTINEL` (optional)

### Create Directory Structure
```bash
# On SD card root
mkdir -p models/s3
```

Note: Model files will be available after first build in `build/espdl_models/`

## 🔨 Step 6: Build and Flash

### Full Build
```bash
# Clean previous builds (if any)
idf.py fullclean

# Build
idf.py build

# Copy model to SD card (if using SD card storage)
# The model will be in build/espdl_models/detect.espdl
cp build/espdl_models/detect.espdl /path/to/sdcard/models/s3/

# Flash
idf.py -p /dev/ttyUSB0 flash

# Monitor output
idf.py -p /dev/ttyUSB0 monitor
```

Replace `/dev/ttyUSB0` with your serial port:
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*`
- Windows: `COM3` or similar

### Exit Monitor
Press `Ctrl+]` to exit monitor

## 🧪 Step 7: Testing

### Initial Boot Test
1. Power on the device
2. Check serial monitor for:
   ```
   AUTONOMOUS DETECTION SECURITY SYSTEM - INITIAL BOOT
   PIR sensor warmup: 60000 ms
   ```
3. Wait for PIR warmup and deep sleep

### Motion Detection Test
1. Trigger PIR sensor (wave hand in front)
2. Watch serial output for:
   - SD card mount
   - Camera initialization
   - Image capture
   - AI detection
   - WiFi connection
   - Telegram notification

3. Check Telegram for alert message with photo

### Verify Cooldown
1. Trigger PIR again immediately
2. Should see: "Still in cooldown period"
3. No notification sent

## 🐛 Troubleshooting

### Build Errors

**Error: esp-dl not found**
```bash
# Update components
idf.py reconfigure
```

**Error: Cannot find credentials.hpp**
```bash
# Copy template
cp main/config/credentials.hpp.template main/config/credentials.hpp
# Then edit with your credentials
```

### Runtime Errors

**SD Card Mount Failed**
- Check SD card is FAT32 formatted
- Verify SD card pins in board_config.hpp
- Try different SD card
- Check SD card is not write-protected

**Camera Init Failed**
- Verify camera connections
- Check pin definitions
- Ensure PSRAM is enabled: `idf.py menuconfig` → `Component config → ESP32S3-Specific → Support for external RAM`

**WiFi Connection Failed**
- Verify SSID and password in credentials.hpp
- Check WiFi signal strength
- Try 2.4GHz network (not 5GHz)

**Telegram Not Working**
- Verify bot token and chat ID
- Test bot manually on Telegram
- Check internet connectivity
- Verify HTTPS is allowed (port 443)

**Detection Not Working**
- Verify model file exists on SD card
- Check PSRAM enabled
- Try increasing detection confidence threshold
- Ensure good lighting conditions

### Getting Help

1. Check serial monitor output (`idf.py monitor`)
2. Review logs for error messages
3. Check [Troubleshooting section in README](README.md#-troubleshooting)
4. Open issue on GitHub with logs

## 🚀 Next Steps

### Optimization
- Adjust detection confidence in `app_config.hpp`
- Tune PIR sensitivity
- Configure cooldown period
- Optimize power consumption

### Customization
- Add more sensors
- Implement image storage
- Add web interface
- Create custom alerts

### Deployment
- Use battery power
- Weatherproof enclosure
- Position for optimal detection
- Set up monitoring dashboard

## 📚 Additional Resources

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP-DL Documentation](https://docs.espressif.com/projects/esp-dl/)
- [Project README](README.md)
- [Contributing Guide](CONTRIBUTING.md)

---

**Need help?** Open an issue or discussion on GitHub!
