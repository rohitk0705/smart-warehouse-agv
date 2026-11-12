#ifndef OBSTACLE_AVOIDANCE_H
#define OBSTACLE_AVOIDANCE_H
void initObstacleAvoidance();
void serviceRadarScan();
void executeBypass();
int readDistanceCM();
void initRadarServo();
void centerRadarServo();
void writeRadarServoAngle(int angle);
String radarZoneLabelForAngle(int angle);
void recordMissionRadarTelemetry(const String&, int, float, bool);
#endif
