# System architecture

The AGV is split into two Wi-Fi-connected controllers:

1. The main ESP32 reads the IR array and RFID reader, controls the motors,
   servo, and buzzer, and exposes the local dashboard.
2. The ESP32-CAM provides `/status`, `/capture`, and `/stream` HTTP endpoints.

The main controller can also publish status to Blynk and log events through a
Google Apps Script endpoint. These integrations are optional and require
user-owned credentials.

The main firmware is divided into managers for motors, line following,
obstacle avoidance, RFID, Wi-Fi, camera communication, Blynk, and Sheets
logging. Shared state and mission transitions are handled by `ATLASCore`.
