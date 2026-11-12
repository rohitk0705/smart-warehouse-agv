#include <Arduino.h>
#include "config.h"
#include "ATLASCore.h"
#include "MotorControl.h"
#include "LineFollower.h"
void initLineFollower() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT, INPUT);
}
void handleLineLost() {
  unsigned long now = millis();
  if (!lineLostActive) {
    lineLostActive = true;
    lineLostSinceMs = now;
  } if (now - lineLostSinceMs >= LINE_FIND_TIMEOUT_MS) {
    stopMotors();
    return;
  } if (lastDirection < 0)drive(-SEARCH_SPEED, SEARCH_SPEED);
  else if (lastDirection > 0)drive(SEARCH_SPEED, -SEARCH_SPEED);
  else drive(-SEARCH_SPEED, SEARCH_SPEED);
}
void followLineLocked() {
  int L = digitalRead(IR_LEFT), C = digitalRead(IR_CENTER), R = digitalRead(IR_RIGHT);
  if (L == HIGH && C == LOW && R == HIGH) {
    lineLostActive = false;
    lastDirection = 0;
    drive(BASE_SPEED, BASE_SPEED);
  } else if (L == LOW && C == HIGH && R == HIGH) {
    lineLostActive = false;
    lastDirection = -1;
    drive(-SHARP_TURN_SPEED, BASE_SPEED);
  } else if (L == LOW && C == LOW && R == HIGH) {
    lineLostActive = false;
    lastDirection = -1;
    drive(TURN_SPEED, BASE_SPEED);
  } else if (L == HIGH && C == HIGH && R == LOW) {
    lineLostActive = false;
    lastDirection = 1;
    drive(BASE_SPEED, -SHARP_TURN_SPEED);
  } else if (L == HIGH && C == LOW && R == LOW) {
    lineLostActive = false;
    lastDirection = 1;
    drive(BASE_SPEED, TURN_SPEED);
  } else if (L == LOW && C == LOW && R == LOW) {
    lineLostActive = false;
    drive(INTERSECTION_SPEED, INTERSECTION_SPEED);
  } else if (L == LOW && C == HIGH && R == LOW) {
    lineLostActive = false;
    drive(INTERSECTION_SPEED, INTERSECTION_SPEED);
  } else handleLineLost();
}
void followLinePID() {
  followLineLocked();
}
