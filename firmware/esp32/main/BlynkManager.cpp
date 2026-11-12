#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include <BlynkSimpleEsp32.h>
#include "ATLASCore.h"
#include "ObstacleAvoidance.h"
#include "GoogleSheets.h"
#include "BlynkManager.h"
void initBlynkManager() {
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(5000);
}
BLYNK_WRITE(VPIN_WAREHOUSE_SELECT) {
  selectedWarehouse = param.asInt();
  Serial.printf("Warehouse selected: %d\n", selectedWarehouse);
  if (selectedWarehouse >= 1 && selectedWarehouse <= 3)setDeliveryStatus("Warehouse Selected");
  else setDeliveryStatus("Waiting");
}
BLYNK_WRITE(VPIN_START_BUTTON) {
  int pressed = param.asInt();
  if (pressed != 1)return;
  if (selectedWarehouse != 0 && currentState == IDLE) {
    deliveryCounter++;
    deliveryStartMillis = millis();
    resetRadarMissionSummary();
    lastUploadStatus = "Disabled";
    lastDriveImageUrl = "-";
    lastCameraStatusForSheet = cameraReady ? "Camera Ready" : "Camera Offline";
    radarHoldCenter = false;
    obstaclePresent = false;
    radarLeftObstacle = radarFrontObstacle = radarRightObstacle = false;
    currentState = MOVING_TO_WAREHOUSE;
    setRobotStatus("Starting...");
    setDeliveryStatus("Package In Transit");
    logDeliveryToSheets(selectedWarehouse, "Started", "-");
    Serial.println("Robot started.");
  }
}
BLYNK_WRITE(VPIN_RETURN_BUTTON) {
  int pressed = param.asInt();
  if (pressed == 1 && currentState == AT_WAREHOUSE) {
    radarHoldCenter = false;
    obstaclePresent = false;
    currentState = RETURNING;
    Serial.println("Returning to start...");
  }
}
void pushStatusToBlynk() {
  if (millis() - lastBlynkStatusPush < 5000)return;
  lastBlynkStatusPush = millis();
}


// Keep Blynk's implementation header in this translation unit only.
void serviceBlynk() {
  Blynk.run();
}

void logBlynkEvent(const char *eventCode, const String &message) {
  Blynk.logEvent(eventCode, message);
}

void writeBlynkString(int virtualPin, const String &value) {
  Blynk.virtualWrite(virtualPin, value);
}
