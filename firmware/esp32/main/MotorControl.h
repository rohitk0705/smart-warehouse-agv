#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
void initMotorControl();
void motorLeft(int speed);
void motorRight(int speed);
void drive(int leftSpeed, int rightSpeed);
void stopMotors();
void buzzerOn();
void buzzerOff();
#endif
