#ifndef ATLAS_CONFIG_H
#define ATLAS_CONFIG_H

// ===================== BLYNK =====================
#define BLYNK_TEMPLATE_ID "TMPL3WqwvNUnG"
#define BLYNK_TEMPLATE_NAME "AGV"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// ===================== WIFI =====================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ===================== GOOGLE APPS SCRIPT =====================
#define GOOGLE_SCRIPT_URL "YOUR_GOOGLE_APPS_SCRIPT_URL"

// ===================== ESP32-CAM =====================
#define CAMERA_IP "192.168.35.112"
#define CAMERA_STATUS_URL "http://192.168.35.112/status"
#define CAMERA_CAPTURE_URL "http://192.168.35.112/capture"
#define CAMERA_STREAM_URL "http://192.168.35.112/stream"
#define CAMERA_HTTP_TIMEOUT_MS 15000UL

// ===================== IR =====================
#define IR_LEFT 36
#define IR_CENTER 39
#define IR_RIGHT 34

// ===================== TB6612FNG =====================
#define AIN1 16
#define AIN2 17
#define PWMA 4
#define BIN1 21
#define BIN2 22
#define PWMB 25
#define STBY 32

// ===================== ULTRASONIC / SERVO / BUZZER =====================
#define TRIG_PIN 27
#define ECHO_PIN 35
#define RADAR_SERVO_PIN 33
#define BUZZER_PIN 14

// ===================== RFID =====================
#define RFID_SS_PIN 26
#define RFID_RST_PIN 13

// ===================== ATLAS LINE FOLLOWING =====================
#define SERVO_CENTER 90
#define SERVO_LEFT 150
#define SERVO_RIGHT 30
#define BASE_SPEED 75
#define TURN_SPEED 30
#define SHARP_TURN_SPEED 90
#define SEARCH_SPEED 60
#define INTERSECTION_SPEED 70
#define OBSTACLE_CM 18
#define LINE_FIND_TIMEOUT_MS 400UL
#define BYPASS_COOLDOWN_MS 3500UL

// ===================== BLYNK VIRTUAL PINS =====================
#define VPIN_WAREHOUSE_SELECT 0
#define VPIN_START_BUTTON 1
#define VPIN_RETURN_BUTTON 2
#define VPIN_ROBOT_STATUS 4
#define VPIN_DELIVERY_STATUS 5
#define VPIN_CAMERA_STATUS 6
#define VPIN_RADAR_STATUS 7

// ===================== RFID UIDs =====================
static const byte UID_START[4] = {0x52, 0x38, 0x97, 0x5D};
static const byte UID_W1[4] = {0x49, 0x19, 0x85, 0xB7};
static const byte UID_W2[4] = {0x49, 0xA9, 0xCD, 0xB7};
static const byte UID_W3[4] = {0x29, 0xF0, 0x82, 0xB7};

#endif
