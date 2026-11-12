#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "ATLASCore.h"
#include "GoogleSheets.h"
void logDeliveryToSheets(int warehouse, String deliveryStatus, String returnStatus) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sheets log skipped: WiFi disconnected.");
    return;
  } HTTPClient http;
  http.begin(GOOGLE_SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");
  String warehouseName = "W" + String(warehouse);
  String durationStr = "-";
  if (deliveryStartMillis > 0 && returnStatus == "Returned")durationStr = String((millis() - deliveryStartMillis) / 1000);
  String obstacleDistance = lastObstacleDistanceForSheet;
  if (obstacleDistance == "-" && lastObstacleDirectionForSheet == "No Obstacle")obstacleDistance = "-";
  String payload = "{";
  payload += "\"deliveryId\":" + String(deliveryCounter) + ",";
  payload += "\"warehouse\":\"" + warehouseName + "\",";
  payload += "\"deliveryStatus\":\"" + deliveryStatus + "\",";
  payload += "\"returnStatus\":\"" + returnStatus + "\",";
  payload += "\"robotStatus\":\"" + lastRobotStatusText + "\",";
  payload += "\"cameraStatus\":\"" + lastCameraStatusForSheet + "\",";
  payload += "\"uploadStatus\":\"" + lastUploadStatus + "\",";
  payload += "\"driveImageUrl\":\"" + lastDriveImageUrl + "\",";
  payload += "\"deliveryDuration\":\"" + durationStr + "\",";
  payload += "\"wifiRSSI\":" + String(WiFi.RSSI()) + ",";
  payload += "\"radarDirection\":\"" + lastRadarDirectionForSheet + "\",";
  payload += "\"obstacleDirection\":\"" + lastObstacleDirectionForSheet + "\",";
  payload += "\"obstacleDistance\":\"" + obstacleDistance + "\",";
  payload += "\"servoAngle\":" + String(lastServoAngleForSheet);
  payload += "}";
  int httpCode = http.POST(payload);
  Serial.printf("Sheets log HTTP response: %d\n", httpCode);
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Sheets response:");
    Serial.println(response);
  } http.end();
}
