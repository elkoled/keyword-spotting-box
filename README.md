# Keyword spotting box

This repo includes CAD and firmware files to build a box that consists of ESP32, microphone and display that detects specific keyword using neural networks.

## Build Firmware
1. Open the project with PlatformIO, the libraries will be downloaded automatically.
2. Edit lines 223 - 228 in include/User_Setup.h to fit your pinout. Any I/O pins can be used.
3. Copy the file **/include/User_Setup** to **/.pio/libdeps/esp32dev/TFT_eSPI**
4. Copy the file **/include/lv_conf.h** to **/.pio/libdeps/esp32dev/lvgl**
5. Build and Flash to ESP32