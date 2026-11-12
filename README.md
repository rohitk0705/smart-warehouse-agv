# Smart Warehouse AGV

An ESP32-based IoT-enabled Autonomous Guided Vehicle (AGV) for smart warehouse
automation. The prototype combines line following, RFID station
identification, obstacle detection, local telemetry, Blynk monitoring, and an
ESP32-CAM for image capture.

## Prototype gallery

| Image | Description |
| --- | --- |
| [Hardware close-up](images/prototype/agv-hardware-closeup.jpg) | ESP32-CAM, ultrasonic sensor, servo, and wiring |
| [Test-track run](images/prototype/agv-test-track.jpg) | AGV carrying sample packages on the line-following track |
| [System overview](images/architecture/system-overview.png) | Annotated hardware architecture |

Additional demonstration photos can be added after obtaining consent from
any identifiable people shown in the frame.

## Features

- Three-sensor line following with the ESP32 DevKit V1 (30-pin)
- TB6612FNG dual-motor control
- MFRC522 RFID checkpoints for warehouse stations
- HC-SR04 radar scanning with servo-assisted obstacle bypass
- Local web dashboard for status and mission controls
- Optional Blynk dashboard and Google Apps Script/Sheets logging
- AI-Thinker ESP32-CAM capture, SD-card storage, and live MJPEG stream

## Repository layout

| Path | Contents |
| --- | --- |
| `firmware/esp32/main/` | Modular vehicle controller firmware |
| `firmware/esp32-cam/main/` | AI-Thinker ESP32-CAM firmware |
| `hardware/` | Wiring, pin mapping, and component notes |
| `documentation/` | Architecture, operating principle, and project report |
| `images/` | Prototype evidence |
| `testing/` | Current test status and verification notes |

## Hardware

The main controller uses an ESP32 DevKit V1 (30-pin), TB6612FNG motor driver,
three-channel IR sensor array, MFRC522 RFID reader, HC-SR04 ultrasonic sensor,
servo, buzzer, and an optional AI-Thinker ESP32-CAM.

See [hardware/pin-mapping.md](hardware/pin-mapping.md) and
[hardware/components.md](hardware/components.md) before wiring the prototype.
The HC-SR04 ECHO signal must be reduced to 3.3 V, and the MFRC522 must be
powered from 3.3 V.

## Firmware setup

1. Install Arduino IDE and the ESP32 Arduino core 3.x.
2. Install the libraries listed in
   [firmware/README.md](firmware/README.md).
3. Open `firmware/esp32/main/ATLAS_AGV.ino` as an Arduino sketch.
4. Replace the `YOUR_*` placeholders in `config.h` with your own configuration.
5. Upload the main controller and then the ESP32-CAM firmware.
6. Confirm the ESP32-CAM address matches `CAMERA_IP` in `config.h`.

The local dashboard is available at `http://<ESP32-IP>/` on the same trusted
network. It has no authentication; do not expose it directly to the internet.

## Security and credentials

Credentials are intentionally replaced with placeholders. Do not commit
Wi-Fi passwords, Blynk authentication tokens, Apps Script URLs, or other
secrets. If a previously used credential was exposed, revoke or rotate it
before deploying the hardware again.

## Current status

This repository documents a working prototype and its firmware. Hardware
results are described in [testing/test-results.md](testing/test-results.md);
additional repeatable measurements and automated tests are still welcome.

## License

This project is released under the MIT License. See [LICENSE](LICENSE).
