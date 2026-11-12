# Test status

The project has been assembled as a physical prototype. The repository
contains the firmware and wiring information used for that prototype, but it
does not yet contain an automated hardware test suite.

Before deployment, verify at minimum:

- Motor direction and emergency stop behavior
- IR line-following behavior on the intended track
- RFID UID detection at each station
- HC-SR04 ECHO level is no higher than 3.3 V
- Obstacle stop, bypass, and line re-entry
- ESP32-CAM `/status`, `/capture`, and `/stream`
- Dashboard access only from the trusted local network
- Operation with cloud integrations disabled and enabled

Record board revisions, library versions, track layout, and observed results
when repeating these tests.
