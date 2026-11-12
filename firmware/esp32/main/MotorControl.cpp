#include <Arduino.h>
#include "config.h"
#include "MotorControl.h"
void initMotorControl() {
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(STBY, OUTPUT);
  digitalWrite(STBY, LOW); digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  ledcAttach(PWMA, 20000, 8); ledcAttach(PWMB, 20000, 8);
  pinMode(BUZZER_PIN, OUTPUT); buzzerOff();
}
void motorLeft(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (speed < 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  } ledcWrite(PWMA, abs(speed));
}
void motorRight(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (speed < 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  } ledcWrite(PWMB, abs(speed));
}
void drive(int l, int r) {
  motorLeft(l);
  motorRight(r);
} void stopMotors() {
  drive(0, 0);
} void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
} void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
}
