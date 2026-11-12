# Pin mapping

## Main ESP32

| Function | GPIO |
| --- | ---: |
| IR left / center / right | 36 / 39 / 34 |
| TB6612 AIN1 / AIN2 / PWMA | 16 / 17 / 4 |
| TB6612 BIN1 / BIN2 / PWMB | 21 / 22 / 25 |
| TB6612 STBY | 32 |
| HC-SR04 TRIG / ECHO | 27 / 35 |
| Radar servo | 33 |
| Buzzer | 14 |
| MFRC522 SS / RST | 26 / 13 |
| MFRC522 SCK / MISO / MOSI | 18 / 19 / 23 |

GPIO 34-39 are input-only. The HC-SR04 ECHO line must use a voltage divider or
level shifter so the ESP32 input never exceeds 3.3 V. Power the MFRC522 from
3.3 V.

## AI-Thinker ESP32-CAM

The camera pin mapping is defined in
`firmware/esp32-cam/main/ATLAS_AGV_CAM_AI_Thinker.ino` and follows the
AI-Thinker OV2640 board layout.
