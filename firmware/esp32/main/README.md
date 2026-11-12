# ATLAS AGV - Modular ESP32 Firmware

This project is a modular split of the ATLAS 30-pin ESP32 AGV firmware.

## Main features
- Locked 3-IR line following
- TB6612FNG motor control
- HC-SR04 obstacle detection
- Servo obstacle scan and smooth bypass/re-entry
- MFRC522 RFID warehouse/start detection
- Blynk dashboard
- Local web dashboard hosted directly on the ESP32 (live telemetry and mission controls)
- ESP32-CAM Wi-Fi capture
- Google Apps Script / Sheets logging

## Project structure
- `ATLAS_AGV.ino` - main controller/state machine
- `config.h` - pins, credentials, thresholds and constants
- `ATLASCore.*` - shared state and status helpers
- `MotorControl.*` - TB6612FNG and buzzer
- `LineFollower.*` - locked 3-IR line following
- `ObstacleAvoidance.*` - HC-SR04, servo and bypass
- `RFIDManager.*` - MFRC522 handling
- `WiFiManager.*` - Wi-Fi/reconnect
- `BlynkManager.*` - Blynk callbacks/status
- `CameraManager.*` - ESP32-CAM HTTP interface
- `GoogleSheets.*` - Apps Script logging

## Important
1. Copy the configuration values into `config.h` and replace every `YOUR_*` placeholder before uploading.
2. HC-SR04 ECHO must use a voltage divider/level shifter to <=3.3 V.
3. RC522 must be powered from 3.3 V.
4. ESP32-CAM must use the configured IP address.
5. Never commit real Wi-Fi passwords, Blynk tokens, or Apps Script URLs.
6. This code uses the ESP32 Arduino Core 3.x LEDC API (`ledcAttach`, `ledcWrite`).


## Local dashboard
1. Connect the ESP32 to the configured Wi-Fi network (`WIFI_SSID` in `config.h`).
2. Find the ESP32 IP address in your router's DHCP/client list. The dashboard uses the ESP32's port 80.
3. Connect a phone or computer to the same Wi-Fi network and open `http://<ESP32-IP>/`.
4. Select W1, W2 or W3 and press **START DELIVERY**. Use **RETURN TO BASE** after the RFID warehouse checkpoint is reached.
5. The page polls `/api/status` every 1.5 seconds. The API is local to the ESP32; no cloud service is needed for the dashboard itself.

The dashboard exposes mission controls on the local network without authentication. Use it only on a trusted, access-controlled Wi-Fi network. The existing Blynk and Google Sheets features remain separate and unchanged.
