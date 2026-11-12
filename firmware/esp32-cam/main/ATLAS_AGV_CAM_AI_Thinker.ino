/*
  ============================================================================
  AI-Thinker ESP32-CAM - AGV Wi-Fi Camera
  ============================================================================
  Board: AI-Thinker ESP32-CAM
  Camera: OV2640

  Same Wi-Fi as the main ESP32.

  Static IP:
    192.168.35.112

  HTTP endpoints:
    GET /status
      -> camera status

    GET /capture
      -> capture JPEG
      -> save to microSD
      -> upload JPEG to Google Drive through Apps Script
      -> return status + Drive URL

    GET /stream
      -> live MJPEG stream

  No UART connection to the main ESP32 is used. Communication is Wi-Fi only.
  ============================================================================ */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "FS.h"
#include "SD_MMC.h"
#include <HTTPClient.h>
#include "mbedtls/base64.h"

// ============================================================================
// WIFI
// ============================================================================

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Static IP for the ESP32-CAM.
// IMPORTANT: Change gateway/subnet if your router uses a different network.
IPAddress local_IP(192, 168, 35, 112);

// ============================================================================
// GOOGLE DRIVE APPS SCRIPT
// ============================================================================

// Replace this with the Web App URL of your IMAGE UPLOAD Apps Script.
const char* IMAGE_UPLOAD_URL =
  "YOUR_IMAGE_UPLOAD_APPS_SCRIPT_URL";

// ============================================================================
// AI-THINKER ESP32-CAM PIN MAP
// ============================================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ============================================================================
// SERVER / STATE
// ============================================================================

httpd_handle_t camera_httpd = NULL;

bool cameraReady = false;
bool sdCardReady = false;

unsigned long imageCounter = 0;
unsigned long lastWiFiCheck = 0;

// ============================================================================
// SD CARD
// ============================================================================

bool initSDCard() {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }

  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return false;
  }

  uint64_t cardSizeMB =
    SD_MMC.cardSize() / (1024ULL * 1024ULL);

  Serial.printf(
    "SD Card initialized: %llu MB\n",
    cardSizeMB
  );

  return true;
}

String saveImageToSD(camera_fb_t* fb) {
  if (!sdCardReady || fb == nullptr) {
    return "";
  }

  imageCounter++;

  String path =
    "/delivery_" +
    String(imageCounter) +
    ".jpg";

  fs::File file =
    SD_MMC.open(
      path.c_str(),
      FILE_WRITE
    );

  if (!file) {
    Serial.println(
      "Failed to open SD file."
    );

    return "";
  }

  size_t written =
    file.write(
      fb->buf,
      fb->len
    );

  file.close();

  if (written != fb->len) {
    Serial.println(
      "Incomplete SD write."
    );

    return "";
  }

  Serial.println(
    "Saved to SD: " + path
  );

  return path;
}

// ============================================================================
// CAMERA
// ============================================================================

bool initCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  }
  else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  esp_err_t err =
    esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf(
      "Camera init failed: 0x%x\n",
      err
    );

    return false;
  }

  sensor_t* sensor =
    esp_camera_sensor_get();

  if (sensor != nullptr) {
    sensor->set_brightness(sensor, 0);
    sensor->set_contrast(sensor, 0);
    sensor->set_saturation(sensor, 0);
  }

  Serial.println(
    "OV2640 camera initialized."
  );

  return true;
}

// ============================================================================
// GOOGLE DRIVE UPLOAD
// ============================================================================

bool uploadImageToDrive(
  camera_fb_t* fb,
  const String& filename,
  String& outDriveUrl
) {
  outDriveUrl = "";

  if (
    fb == nullptr ||
    WiFi.status() != WL_CONNECTED
  ) {
    return false;
  }

  if (
    String(IMAGE_UPLOAD_URL).startsWith("YOUR_")
  ) {
    Serial.println(
      "ERROR: IMAGE_UPLOAD_URL is still a placeholder."
    );

    return false;
  }

  // Base64 size for the JPEG.
  size_t encodedLen =
    4 * ((fb->len + 2) / 3) + 1;

  char* encodedBuf =
    (char*)malloc(encodedLen);

  if (encodedBuf == nullptr) {
    Serial.println(
      "Base64 buffer allocation failed."
    );

    return false;
  }

  size_t outLen = 0;

  int base64Result =
    mbedtls_base64_encode(
      (unsigned char*)encodedBuf,
      encodedLen,
      &outLen,
      fb->buf,
      fb->len
    );

  if (base64Result != 0) {
    free(encodedBuf);

    Serial.printf(
      "Base64 encode failed: %d\n",
      base64Result
    );

    return false;
  }

  encodedBuf[outLen] = '\0';

  String payload;
  payload.reserve(
    outLen + filename.length() + 100
  );

  payload = "{";
  payload +=
    "\"filename\":\"" +
    filename +
    "\",";
  payload +=
    "\"mimeType\":\"image/jpeg\",";
  payload +=
    "\"data\":\"" +
    String(encodedBuf) +
    "\"";
  payload += "}";

  free(encodedBuf);

  const int MAX_ATTEMPTS = 3;

  for (
    int attempt = 1;
    attempt <= MAX_ATTEMPTS;
    attempt++
  ) {
    Serial.printf(
      "Drive upload attempt %d/%d...\n",
      attempt,
      MAX_ATTEMPTS
    );

    HTTPClient http;

    http.setConnectTimeout(5000);
    http.setTimeout(20000);

    if (
      !http.begin(
        IMAGE_UPLOAD_URL
      )
    ) {
      Serial.println(
        "HTTP begin failed."
      );

      if (attempt < MAX_ATTEMPTS) {
        delay(1000);
        continue;
      }

      return false;
    }

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    int httpCode =
      http.POST(payload);

    String response =
      http.getString();

    http.end();

    Serial.printf(
      "Drive upload HTTP response: %d\n",
      httpCode
    );

    if (httpCode >= 200 &&
        httpCode < 300) {

      Serial.println(
        "Drive upload response:"
      );

      Serial.println(
        response
      );

      // Expected:
      // {"success":true,"fileUrl":"..."}
      int urlKeyPos =
        response.indexOf(
          "\"fileUrl\":\""
        );

      if (urlKeyPos >= 0) {
        int start =
          urlKeyPos +
          strlen("\"fileUrl\":\"");

        int end =
          response.indexOf(
            "\"",
            start
          );

        if (end > start) {
          outDriveUrl =
            response.substring(
              start,
              end
            );
        }
      }

      // Apps Script may return success without a URL.
      if (
        response.indexOf(
          "\"success\":true"
        ) >= 0
      ) {
        return true;
      }

      if (
        outDriveUrl.length() > 0
      ) {
        return true;
      }
    }

    if (
      attempt < MAX_ATTEMPTS
    ) {
      delay(1000);
    }
  }

  return false;
}

// ============================================================================
// CAPTURE + SAVE + UPLOAD
// ============================================================================

camera_fb_t* captureWithRetry() {
  camera_fb_t* fb =
    esp_camera_fb_get();

  if (fb != nullptr) {
    return fb;
  }

  Serial.println(
    "Capture failed. Retrying..."
  );

  delay(100);

  return esp_camera_fb_get();
}

String performCapture() {
  camera_fb_t* fb =
    captureWithRetry();

  if (fb == nullptr) {
    return "CAPTURE_FAILED";
  }

  Serial.printf(
    "Captured image: %u bytes\n",
    fb->len
  );

  String savedPath =
    saveImageToSD(fb);

  String response =
    "CAPTURE_OK\n";

  if (savedPath.length() > 0) {
    response +=
      "SD_SAVE_SUCCESS\n";
  }
  else {
    response +=
      "SD_SAVE_FAILED\n";
  }

  String filename;

  if (savedPath.length() > 0) {
    filename =
      savedPath.substring(1);
  }
  else {
    filename =
      "delivery_" +
      String(millis()) +
      ".jpg";
  }

  Serial.println(
    "Starting Google Drive upload..."
  );

  response +=
    "UPLOAD_STARTED\n";

  String driveUrl;

  bool uploaded =
    uploadImageToDrive(
      fb,
      filename,
      driveUrl
    );

  esp_camera_fb_return(fb);

  if (uploaded) {
    response +=
      "UPLOAD_SUCCESS\n";

    if (driveUrl.length() > 0) {
      response +=
        "DRIVE_URL:" +
        driveUrl +
        "\n";
    }
  }
  else {
    response +=
      "UPLOAD_FAILED\n";
  }

  return response;
}

// ============================================================================
// HTTP HANDLERS
// ============================================================================

static esp_err_t status_handler(
  httpd_req_t* req
) {
  String json = "{";

  json +=
    "\"camera\":\"" +
    String(
      cameraReady
      ? "ready"
      : "error"
    ) +
    "\",";

  json +=
    "\"wifi\":\"" +
    String(
      WiFi.status() ==
      WL_CONNECTED
      ? "connected"
      : "disconnected"
    ) +
    "\",";

  json +=
    "\"sd\":\"" +
    String(
      sdCardReady
      ? "mounted"
      : "not_mounted"
    ) +
    "\",";

  json +=
    "\"stream\":\"available\"";

  json += "}";

  httpd_resp_set_type(
    req,
    "application/json"
  );

  return httpd_resp_send(
           req,
           json.c_str(),
           json.length()
         );
}

static esp_err_t capture_handler(
  httpd_req_t* req
) {
  String result =
    performCapture();

  httpd_resp_set_type(
    req,
    "text/plain"
  );

  return httpd_resp_send(
           req,
           result.c_str(),
           result.length()
         );
}

#define PART_BOUNDARY \
  "123456789000000000000987654321"

static const char*
STREAM_CONTENT_TYPE =
  "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

static const char*
STREAM_BOUNDARY =
  "\r\n--" PART_BOUNDARY "\r\n";

static const char*
STREAM_PART =
  "Content-Type: image/jpeg\r\n"
  "Content-Length: %u\r\n\r\n";

static esp_err_t stream_handler(
  httpd_req_t* req
) {
  camera_fb_t* fb = nullptr;
  esp_err_t res = ESP_OK;
  char partBuf[64];

  res =
    httpd_resp_set_type(
      req,
      STREAM_CONTENT_TYPE
    );

  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb =
      esp_camera_fb_get();

    if (fb == nullptr) {
      Serial.println(
        "Stream capture failed."
      );

      res = ESP_FAIL;
    }
    else {
      size_t headerLen =
        snprintf(
          partBuf,
          sizeof(partBuf),
          STREAM_PART,
          fb->len
        );

      res =
        httpd_resp_send_chunk(
          req,
          STREAM_BOUNDARY,
          strlen(STREAM_BOUNDARY)
        );

      if (res == ESP_OK) {
        res =
          httpd_resp_send_chunk(
            req,
            partBuf,
            headerLen
          );
      }

      if (res == ESP_OK) {
        res =
          httpd_resp_send_chunk(
            req,
            (const char*)fb->buf,
            fb->len
          );
      }
    }

    if (fb != nullptr) {
      esp_camera_fb_return(fb);
      fb = nullptr;
    }

    if (res != ESP_OK) {
      break;
    }
  }

  return res;
}

void startCameraServer() {
  httpd_config_t config =
    HTTPD_DEFAULT_CONFIG();

  config.server_port = 80;
  config.max_uri_handlers = 8;

  httpd_uri_t statusUri = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = status_handler,
    .user_ctx = NULL
  };

  httpd_uri_t captureUri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = capture_handler,
    .user_ctx = NULL
  };

  httpd_uri_t streamUri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (
    httpd_start(
      &camera_httpd,
      &config
    ) == ESP_OK
  ) {
    httpd_register_uri_handler(
      camera_httpd,
      &statusUri
    );

    httpd_register_uri_handler(
      camera_httpd,
      &captureUri
    );

    httpd_register_uri_handler(
      camera_httpd,
      &streamUri
    );

    Serial.println(
      "Camera HTTP server started."
    );
  }
}

// ============================================================================
// WIFI
// ============================================================================

void connectWiFi() {
  if (!WiFi.config(
        local_IP
      )) {
    Serial.println(
      "Static IP configuration failed."
    );
  }

  WiFi.begin(
    ssid,
    password
  );

  Serial.print(
    "Connecting to WiFi"
  );

  unsigned long start =
    millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - start < 20000
  ) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (
    WiFi.status() ==
    WL_CONNECTED
  ) {
    Serial.print(
      "ESP32-CAM IP: "
    );

    Serial.println(
      WiFi.localIP()
    );
  }
  else {
    Serial.println(
      "WiFi connection failed."
    );
  }
}

void checkWiFiReconnect() {
  if (
    millis() - lastWiFiCheck <
    5000
  ) {
    return;
  }

  lastWiFiCheck =
    millis();

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {
    Serial.println(
      "CAM WiFi disconnected. Reconnecting..."
    );

    WiFi.disconnect();
    WiFi.begin(
      ssid,
      password
    );
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println(
    "=========================================="
  );
  Serial.println(
    "AI-Thinker ESP32-CAM - AGV CAMERA"
  );
  Serial.println(
    "=========================================="
  );

  connectWiFi();

  if (
    WiFi.status() !=
    WL_CONNECTED
  ) {
    Serial.println(
      "Camera will retry WiFi in loop."
    );
  }

  sdCardReady =
    initSDCard();

  cameraReady =
    initCamera();

  if (!cameraReady) {
    Serial.println(
      "Camera initialization failed."
    );
  }

  if (
    cameraReady &&
    WiFi.status() ==
    WL_CONNECTED
  ) {
    startCameraServer();

    Serial.println();
    Serial.println(
      "ESP32-CAM READY"
    );

    Serial.println(
      "Status:  http://192.168.35.112/status"
    );

    Serial.println(
      "Capture: http://192.168.35.112/capture"
    );

    Serial.println(
      "Stream:  http://192.168.35.112/stream"
    );
  }
}

// ============================================================================
// LOOP
// ============================================================================

void loop() {
  checkWiFiReconnect();
  delay(10);
}

/*
  ============================================================================
  WIRING
  ============================================================================

  NO DATA WIRE IS REQUIRED BETWEEN THE TWO ESP32 BOARDS.

  Main ESP32:
      Wi-Fi -> same network

  AI-Thinker ESP32-CAM:
      Wi-Fi -> same network
      Static IP -> 192.168.35.112

  Main ESP32 requests:
      GET http://192.168.35.112/status
      GET http://192.168.35.112/capture
      GET http://192.168.35.112/stream

  microSD:
      Insert FAT32 microSD card into the ESP32-CAM slot.

  Power:
      Use a stable 5 V supply capable of handling ESP32-CAM current peaks.

  ============================================================================
*/
