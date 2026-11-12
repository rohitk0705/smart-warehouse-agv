#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "ATLASCore.h"
#include "CameraManager.h"
void initCameraManager() {
  cameraReady = checkCameraStatus();
}
bool checkCameraStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    cameraReady = false;
    setCameraStatus("WiFi Offline");
    return false;
  } HTTPClient http;
  http.setConnectTimeout(1500);
  http.setTimeout(2500);
  if (!http.begin(CAMERA_STATUS_URL)) {
    cameraReady = false;
    setCameraStatus("Camera Offline");
    return false;
  } int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    response.trim();
    cameraReady = response.indexOf("\"camera\":\"ready\"") >= 0;
    if (cameraReady) {
      setCameraStatus("Camera Ready");
      Serial.println("[ESP32-CAM] " + response);
    } else setCameraStatus("Camera Error");
    http.end();
    return cameraReady;
  } http.end();
  cameraReady = false;
  setCameraStatus("Camera Offline");
  return false;
}
bool requestCameraCapture() {
  if (WiFi.status() != WL_CONNECTED) {
    lastUploadStatus = "WiFi Offline";
    lastDriveImageUrl = "-";
    setCameraStatus("WiFi Offline");
    return false;
  } if (!cameraReady && !checkCameraStatus()) {
    lastUploadStatus = "Camera Offline";
    lastDriveImageUrl = "-";
    return false;
  } HTTPClient http;
  http.setConnectTimeout(3000);
  http.setTimeout(CAMERA_HTTP_TIMEOUT_MS);
  Serial.println("[ESP32-CAM] Requesting capture...");
  setCameraStatus("Capturing Image");
  lastUploadStatus = "Capturing";
  lastDriveImageUrl = "-";
  if (!http.begin(CAMERA_CAPTURE_URL)) {
    lastUploadStatus = "HTTP Begin Failed";
    setCameraStatus("Capture Failed");
    return false;
  } int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[ESP32-CAM] Capture HTTP error: %d\n", httpCode);
    http.end();
    lastUploadStatus = "HTTP Error " + String(httpCode);
    setCameraStatus("Capture Failed");
    cameraReady = false;
    return false;
  } String response = http.getString();
  http.end();
  response.trim();
  Serial.println("[ESP32-CAM] Capture response:");
  Serial.println(response);
  bool captureOK = response.indexOf("CAPTURE_OK") >= 0;
  bool uploadOK = response.indexOf("UPLOAD_SUCCESS") >= 0;
  bool uploadFailed = response.indexOf("UPLOAD_FAILED") >= 0;
  bool sdOK = response.indexOf("SD_SAVE_SUCCESS") >= 0;
  int urlPos = response.indexOf("DRIVE_URL:");
  if (urlPos >= 0) {
    String url = response.substring(urlPos + strlen("DRIVE_URL:"));
    int newlinePos = url.indexOf('\n');
    if (newlinePos >= 0)url = url.substring(0, newlinePos);
    url.trim();
    if (url.length() > 0)lastDriveImageUrl = url;
  } if (uploadOK) {
    lastUploadStatus = "Upload Success";
    setCameraStatus("Image Uploaded");
  } else if (uploadFailed) {
    lastUploadStatus = "Upload Failed";
    setCameraStatus("Drive Upload Failed");
  } else if (sdOK) {
    lastUploadStatus = "Saved to SD";
    setCameraStatus("Image Saved to SD");
  } if (!captureOK) {
    setCameraStatus("Capture Failed");
    return false;
  } return true;
}
