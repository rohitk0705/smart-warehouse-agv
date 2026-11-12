# ATLAS 30-Pin Wiring

| Function | GPIO |
|---|---:|
| IR Left | 36 (VP) |
| IR Center | 39 (VN) |
| IR Right | 34 |
| TB6612 AIN1 | 16 |
| TB6612 AIN2 | 17 |
| TB6612 PWMA | 4 |
| TB6612 BIN1 | 21 |
| TB6612 BIN2 | 22 |
| TB6612 PWMB | 25 |
| TB6612 STBY | 32 |
| HC-SR04 TRIG | 27 |
| HC-SR04 ECHO | 35 (voltage divider required) |
| Servo signal | 33 |
| Buzzer | 14 |
| MFRC522 SS/SDA | 26 |
| MFRC522 RST | 13 |
| MFRC522 SCK | 18 |
| MFRC522 MISO | 19 |
| MFRC522 MOSI | 23 |

RC522 VCC = 3.3 V. HC-SR04 ECHO must be reduced to <=3.3 V before the ESP32 GPIO.
