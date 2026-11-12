  #ifndef ATLAS_CORE_H
#define ATLAS_CORE_H
#include <Arduino.h>

enum RobotState { IDLE, MOVING_TO_WAREHOUSE, OBSTACLE_STOPPED, AT_WAREHOUSE, RETURNING, ARRIVED_HOME };

extern RobotState currentState;
extern int selectedWarehouse;
extern bool obstaclePresent;
extern bool cameraReady;
extern bool radarHoldCenter;
extern int radarCurrentAngle;
extern String radarCurrentZoneLabel;
extern int lastDirection;
extern unsigned long lineLostSinceMs;
extern bool lineLostActive;
extern unsigned long bypassCooldownUntil;
extern unsigned long lastBlynkStatusPush;
extern unsigned long lastWiFiCheck;
extern unsigned long lastCameraCheck;
extern String lastRobotStatusText;
extern String lastDeliveryStatusText;
extern String lastCameraStatusText;
extern String lastRadarStatusText;
extern String lastRadarDirectionForSheet;
extern String lastObstacleDirectionForSheet;
extern String lastObstacleDistanceForSheet;
extern int lastServoAngleForSheet;
extern bool radarLeftObstacle, radarFrontObstacle, radarRightObstacle;
extern float radarLeftDistanceCm, radarFrontDistanceCm, radarRightDistanceCm;
extern int deliveryCounter;
extern unsigned long deliveryStartMillis;
extern String lastDriveImageUrl, lastUploadStatus, lastCameraStatusForSheet;

void initATLASCore();
void setRobotStatus(const String &text);
void setDeliveryStatus(const String &text);
void setCameraStatus(const String &text);
void setRadarStatus(const String &text);
void resetRadarMissionSummary();

#endif
