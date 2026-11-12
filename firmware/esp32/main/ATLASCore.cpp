#include "ATLASCore.h"
#include "config.h"
#include "BlynkManager.h"

RobotState currentState = IDLE;
int selectedWarehouse = 0;
bool obstaclePresent = false;
bool cameraReady = false;
bool radarHoldCenter = true;
int radarCurrentAngle = SERVO_CENTER;
String radarCurrentZoneLabel = "Front";
int lastDirection = 0;
unsigned long lineLostSinceMs = 0;
bool lineLostActive = false;
unsigned long bypassCooldownUntil = 0;
unsigned long lastBlynkStatusPush = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastCameraCheck = 0;
String lastRobotStatusText = "";
String lastDeliveryStatusText = "";
String lastCameraStatusText = "";
String lastRadarStatusText = "";
String lastRadarDirectionForSheet = "No Obstacle";
String lastObstacleDirectionForSheet = "No Obstacle";
String lastObstacleDistanceForSheet = "-";
int lastServoAngleForSheet = SERVO_CENTER;
bool radarLeftObstacle = false, radarFrontObstacle = false, radarRightObstacle = false;
float radarLeftDistanceCm = -1.0, radarFrontDistanceCm = -1.0, radarRightDistanceCm = -1.0;
int deliveryCounter = 0;
unsigned long deliveryStartMillis = 0;
String lastDriveImageUrl = "-";
String lastUploadStatus = "-";
String lastCameraStatusForSheet = "Camera Disabled";

void initATLASCore() {
  resetRadarMissionSummary();
}

void setRobotStatus(const String &text) {
  if (text == lastRobotStatusText) return;
  lastRobotStatusText = text;
  writeBlynkString(VPIN_ROBOT_STATUS, text);
  Serial.println("[Robot Status] " + text);
}
void setDeliveryStatus(const String &text) {
  if (text == lastDeliveryStatusText) return;
  lastDeliveryStatusText = text;
  writeBlynkString(VPIN_DELIVERY_STATUS, text);
  Serial.println("[Delivery Status] " + text);
}
void setCameraStatus(const String &text) {
  if (text == lastCameraStatusText) return;
  lastCameraStatusText = text;
  lastCameraStatusForSheet = text;
  writeBlynkString(VPIN_CAMERA_STATUS, text);
  Serial.println("[Camera Status] " + text);
}
void setRadarStatus(const String &text) {
  if (text == lastRadarStatusText) return;
  lastRadarStatusText = text;
  writeBlynkString(VPIN_RADAR_STATUS, text);
  Serial.println("[Radar Status] " + text);
}
void resetRadarMissionSummary() {
  lastRadarDirectionForSheet = "No Obstacle";
  lastObstacleDirectionForSheet = "No Obstacle";
  lastObstacleDistanceForSheet = "-";
  lastServoAngleForSheet = SERVO_CENTER;
  radarLeftObstacle = radarFrontObstacle = radarRightObstacle = false;
  radarLeftDistanceCm = radarFrontDistanceCm = radarRightDistanceCm = -1.0;
}
