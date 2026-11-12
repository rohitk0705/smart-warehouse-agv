#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "BlynkManager.h"
#include "ATLASCore.h"
#include "MotorControl.h"
#include "ObstacleAvoidance.h"
static Servo radarServo;
String radarZoneLabelForAngle(int angle) {
  if (angle <= 75)return "Left";
  if (angle >= 105)return "Right";
  return "Front";
}
void writeRadarServoAngle(int angle) {
  angle = constrain(angle, 30, 150);
  radarCurrentAngle = angle;
  radarCurrentZoneLabel = radarZoneLabelForAngle(angle);
  radarServo.write(angle);
  lastServoAngleForSheet = angle;
}
void centerRadarServo() {
  writeRadarServoAngle(SERVO_CENTER);
  lastRadarDirectionForSheet = "Front";
  lastServoAngleForSheet = SERVO_CENTER;
}
void initRadarServo() {
  radarServo.setPeriodHertz(50);
  radarServo.attach(RADAR_SERVO_PIN, 500, 2400);
  centerRadarServo();
}
void initObstacleAvoidance() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  initRadarServo();
}
void recordMissionRadarTelemetry(const String &zoneLabel, int angle, float distanceCm, bool obstacleDetected) {
  lastRadarDirectionForSheet = zoneLabel;
  lastServoAngleForSheet = angle;
  if (zoneLabel == "Left") {
    radarLeftDistanceCm = distanceCm;
    radarLeftObstacle = obstacleDetected;
  } else if (zoneLabel == "Right") {
    radarRightDistanceCm = distanceCm;
    radarRightObstacle = obstacleDetected;
  } else {
    radarFrontDistanceCm = distanceCm;
    radarFrontObstacle = obstacleDetected;
  } if (obstacleDetected) {
    lastObstacleDirectionForSheet = zoneLabel;
    lastObstacleDistanceForSheet = String(distanceCm, 1);
  }
}
int readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0)return 400;
  int distance = (int)(duration * 0.0343f / 2.0f);
  if (distance < 2 || distance > 400)return 400;
  return distance;
}
void executeBypass() {
  Serial.println("OBSTACLE CONFIRMED: Executing Smooth Overtake");
  setRobotStatus("Bypassing Obstacle");
  setRadarStatus("Bypass in Progress");
  drive(-120, -120);
  delay(350);
  stopMotors();
  delay(200);
  drive(-165, 165);
  delay(420);
  stopMotors();
  delay(200);
  drive(130, 130);
  delay(1100);
  stopMotors();
  delay(200);
  drive(165, -165);
  delay(480);
  stopMotors();
  delay(200);
  Serial.println("Approaching line...");
  setRobotStatus("Searching for Track");
  drive(90, 90);
  unsigned long searchStart = millis();
  while (digitalRead(IR_LEFT) == HIGH && digitalRead(IR_CENTER) == HIGH && digitalRead(IR_RIGHT) == HIGH) {
    if (millis() - searchStart > 5000) {
      stopMotors();
      buzzerOn();
      obstaclePresent = true;
      Serial.println("Line not found. Safe Stop.");
      setRobotStatus("Line Not Found - Safe Stop");
      while (true) {
        serviceBlynk();
        delay(1000);
      }
    } delay(2);
  } drive(-100, -100);
  delay(40);
  stopMotors();
  delay(150);
  Serial.println("Line Touched! Merging smoothly...");
  buzzerOff();
  radarServo.write(SERVO_CENTER);
  radarCurrentAngle = SERVO_CENTER;
  radarCurrentZoneLabel = "Front";
  lastDirection = 0;
  lineLostActive = false;
  obstaclePresent = false;
  bypassCooldownUntil = millis() + BYPASS_COOLDOWN_MS;
  radarLeftObstacle = radarFrontObstacle = radarRightObstacle = false;
  setRadarStatus("Track Re-entry Complete");
}
void serviceRadarScan() {
  if (currentState != MOVING_TO_WAREHOUSE && currentState != RETURNING) {
    centerRadarServo();
    return;
  } unsigned long now = millis();
  if (now < bypassCooldownUntil) {
    centerRadarServo();
    return;
  } centerRadarServo();
  float distance = readDistanceCM();
  if (distance <= OBSTACLE_CM && distance > 0) {
    delay(15);
    float confirmDistance = readDistanceCM();
    if (confirmDistance <= OBSTACLE_CM + 5) {
      stopMotors();
      obstaclePresent = true;
      buzzerOn();
      radarFrontObstacle = true;
      radarFrontDistanceCm = confirmDistance;
      recordMissionRadarTelemetry("Front", SERVO_CENTER, confirmDistance, true);
      setRobotStatus("Obstacle Ahead");
      setRadarStatus("Obstacle Ahead");
      logBlynkEvent("obstacle_detected", "Obstacle detected - robot paused");
      radarServo.write(SERVO_LEFT);
      radarCurrentAngle = SERVO_LEFT;
      radarCurrentZoneLabel = "Left";
      lastServoAngleForSheet = SERVO_LEFT;
      delay(350);
      float leftDistance = readDistanceCM();
      recordMissionRadarTelemetry("Left", SERVO_LEFT, leftDistance, leftDistance <= OBSTACLE_CM);
      radarServo.write(SERVO_RIGHT);
      radarCurrentAngle = SERVO_RIGHT;
      radarCurrentZoneLabel = "Right";
      lastServoAngleForSheet = SERVO_RIGHT;
      delay(450);
      float rightDistance = readDistanceCM();
      recordMissionRadarTelemetry("Right", SERVO_RIGHT, rightDistance, rightDistance <= OBSTACLE_CM);
      radarServo.write(SERVO_CENTER);
      radarCurrentAngle = SERVO_CENTER;
      radarCurrentZoneLabel = "Front";
      lastServoAngleForSheet = SERVO_CENTER;
      delay(250);
      float recheckDist = readDistanceCM();
      if (recheckDist > OBSTACLE_CM + 5) {
        obstaclePresent = false;
        radarFrontObstacle = false;
        buzzerOff();
        Serial.println("Obstacle removed - resuming normal tracking.");
        setRobotStatus("Obstacle Removed");
        setRadarStatus("No Obstacle");
        bypassCooldownUntil = millis() + 1500;
        return;
      } executeBypass();
      return;
    }
  } radarFrontObstacle = false;
  radarLeftObstacle = false;
  radarRightObstacle = false;
  lastRadarDirectionForSheet = "Front";
  lastServoAngleForSheet = SERVO_CENTER;
  setRadarStatus("No Obstacle");
}
