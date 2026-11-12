#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <Arduino.h>
#include "config.h"
#include "ATLASCore.h"
#include "MotorControl.h"
#include "LineFollower.h"
#include "ObstacleAvoidance.h"
#include "RFIDManager.h"
#include "WiFiManager.h"
#include "CameraManager.h"
#include "BlynkManager.h"

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  initATLASCore();
  initLineFollower();
  initMotorControl();
  initObstacleAvoidance();
  initRFIDManager();
  initWiFiManager();
  initBlynkManager();
  cameraReady = checkCameraStatus();
  digitalWrite(STBY, HIGH);
  stopMotors();
  delay(300);
  currentState = IDLE;
  Serial.println();
  Serial.println("==========================================");
  Serial.println("ATLAS - IoT Smart Warehouse AGV");
  Serial.println("ESP32 DevKit V1 - 30 Pin");
  Serial.println("3-IR Locked Line Following");
  Serial.println("Precision Track Re-entry + Bypass");
  Serial.println("==========================================");
  Serial.println("System Ready.");
  Serial.println("Local Dashboard: http://<ESP32-IP>/ (check router DHCP clients or Serial Monitor)");
  setRobotStatus("Idle");
  setDeliveryStatus("Waiting");
  setCameraStatus(cameraReady ? "Camera Ready" : "Camera Offline");
  setRadarStatus("No Obstacle");
}

void loop() {
  serviceBlynk();
  serviceLocalDashboard();
  checkWiFiReconnect();
  if (millis() - lastCameraCheck >= 10000) {
    lastCameraCheck = millis();
    cameraReady = checkCameraStatus();
  }
  serviceRadarScan();
  pushStatusToBlynk();
  switch (currentState) {
    case IDLE:
      stopMotors();
      radarHoldCenter = true;
      centerRadarServo();
      setRobotStatus("Idle");
      break;
    case MOVING_TO_WAREHOUSE:
      radarHoldCenter = false;
      if (obstaclePresent) {
        stopMotors();
        setRobotStatus("Obstacle Ahead");
        break;
      }
      followLineLocked();
      checkRFIDForWarehouse();
      break;
    case OBSTACLE_STOPPED:
      stopMotors();
      radarHoldCenter = true;
      centerRadarServo();
      setRobotStatus("Obstacle Ahead");
      if (!obstaclePresent) currentState = MOVING_TO_WAREHOUSE;
      break;
    case AT_WAREHOUSE:
      stopMotors();
      radarHoldCenter = true;
      centerRadarServo();
      setRobotStatus("At Warehouse");
      break;
    case RETURNING:
      radarHoldCenter = false;
      if (obstaclePresent) {
        stopMotors();
        setRobotStatus("Obstacle Ahead");
        break;
      }
      followLineLocked();
      checkRFIDForStart();
      break;
    case ARRIVED_HOME:
      stopMotors();
      radarHoldCenter = true;
      centerRadarServo();
      setRobotStatus("Mission Complete");
      setDeliveryStatus("Ready for Next Delivery");
      currentState = IDLE;
      break;
  }
  delay(2);
}
