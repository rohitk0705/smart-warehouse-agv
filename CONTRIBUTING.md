# Contributing

Thanks for contributing to Smart Warehouse AGV.

## Before opening a pull request

- Explain the hardware or firmware change and how it was tested.
- Update wiring, configuration, or documentation when behavior changes.
- Keep credentials, tokens, private URLs, and personal data out of commits.
- Do not change GPIO assignments without updating the pin-mapping document.
- Prefer small, focused pull requests.

## Firmware changes

Test firmware changes on the target ESP32 board where possible. Record the
board, Arduino-ESP32 core version, libraries, and observed result in the pull
request description.
