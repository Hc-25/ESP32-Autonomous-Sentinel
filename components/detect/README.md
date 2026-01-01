# Detect Component

Generic object detection component for ESP-DL framework.

## Overview

This component provides a simplified API for object detection using ESP-DL models. It abstracts the complexity of model loading, preprocessing, and inference.

## Features

- Model loading from flash (RODATA/partition) or SD card
- Automatic model selection based on target (ESP32-S3, ESP32-P4)
- Configurable detection thresholds
- Support for PICO detection models

## Usage

```cpp
#include "detect.hpp"

// Create detector instance
Detect detector;

// Run detection on image
auto& results = detector.run(image);

// Process results
for (const auto& detection : results) {
    printf("Score: %.3f, Box: [%d, %d, %d, %d]\n",
           detection.score,
           static_cast<int>(detection.box[0]),
           static_cast<int>(detection.box[1]),
           static_cast<int>(detection.box[2]),
           static_cast<int>(detection.box[3]));
}
```

## Configuration

Use `idf.py menuconfig` to configure:

- **Component config → models: detect**
  - Model selection (PICO_S8_V1)
  - Model location (Flash RODATA, Flash Partition, SD Card)
  - SD card model directory

## Model Files

Place model files in:
- For ESP32-S3: `models/s3/detect_pico_s8_v1.espdl`
- For ESP32-P4: `models/p4/detect_pico_s8_v1.espdl`

## Dependencies

- ESP-DL library (v3.2.0 or higher)

## License

MIT License - See LICENSE file for details
