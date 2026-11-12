# Working principle

1. On startup, the main ESP32 initializes the sensors, motor driver, dashboard,
   and network services.
2. The three IR sensors provide line position feedback while the controller
   adjusts the left and right motor speeds.
3. RFID tags identify the start point and warehouse stations.
4. The ultrasonic sensor scans from a servo-mounted position. When an obstacle
   is detected, the vehicle stops and can perform a bypass and line
   re-entry sequence.
5. The selected delivery station is reported through the local dashboard and
   optional Blynk service.
6. The ESP32-CAM captures images on request, stores them on microSD, and can
   optionally upload them through a user-owned Apps Script endpoint.
