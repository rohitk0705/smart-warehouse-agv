#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include "BlynkManager.h"
#include "ATLASCore.h"
#include "MotorControl.h"
#include "ObstacleAvoidance.h"
#include "CameraManager.h"
#include "GoogleSheets.h"
#include "RFIDManager.h"
static MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
static bool matchUID(const byte *scanned, byte scannedLen, const byte *known, byte knownLen) {
  if (scannedLen != knownLen)return false;
  for (byte i = 0; i < knownLen; i++)if (scanned[i] != known[i])return false;
  return true;
}
void initRFIDManager() {
  SPI.begin();
  rfid.PCD_Init();
  delay(50);
}
void checkRFIDForWarehouse() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())return;
  byte len = rfid.uid.size;
  byte *scanned = rfid.uid.uidByte;
  bool isTarget = false;
  if (selectedWarehouse == 1 && matchUID(scanned, len, UID_W1, 4))isTarget = true;
  if (selectedWarehouse == 2 && matchUID(scanned, len, UID_W2, 4))isTarget = true;
  if (selectedWarehouse == 3 && matchUID(scanned, len, UID_W3, 4))isTarget = true;
  if (isTarget) {
    stopMotors();
    currentState = AT_WAREHOUSE;
    Serial.printf("Arrived at Warehouse %d\n", selectedWarehouse);
    setRobotStatus("RFID Detected");
    setDeliveryStatus("Package Delivered");
    logBlynkEvent("delivery_completed", String("Delivery completed at Warehouse ") + selectedWarehouse);
    radarHoldCenter = true;
    centerRadarServo();
    bool cameraCaptured = requestCameraCapture();
    if (!cameraCaptured)setCameraStatus(cameraReady ? "Capture Failed" : "Camera Offline");
    logDeliveryToSheets(selectedWarehouse, "Delivered", "Pending");
    setDeliveryStatus(cameraCaptured ? "Delivered + Image Captured" : "Delivered");
  } rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
void checkRFIDForStart() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())return;
  byte len = rfid.uid.size;
  byte *scanned = rfid.uid.uidByte;
  if (matchUID(scanned, len, UID_START, 4)) {
    stopMotors();
    currentState = ARRIVED_HOME;
    Serial.println("Returned to Start.");
    radarHoldCenter = true;
    centerRadarServo();
    setDeliveryStatus("Ready for Next Delivery");
    logBlynkEvent("return_completed", "Robot returned to start point");
    logDeliveryToSheets(selectedWarehouse, "Delivered", "Returned");
    selectedWarehouse = 0;
  } rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
