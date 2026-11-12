# Firmware

## Main ESP32 controller

Open `esp32/main/ATLAS_AGV.ino` in Arduino IDE. The other files in that
directory are included by the sketch and should remain together.

Required libraries:

- Blynk
- MFRC522
- ESP32 Arduino core 3.x

The ESP32 core provides Wi-Fi, HTTP, SPI, servo, and camera communication
support used by this project.

## ESP32-CAM

Open `esp32-cam/main/ATLAS_AGV_CAM_AI_Thinker.ino` with the AI-Thinker
ESP32-CAM board selected. Replace the Wi-Fi and Apps Script placeholders before
uploading.
