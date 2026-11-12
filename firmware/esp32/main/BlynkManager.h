#ifndef BLYNK_MANAGER_H
#define BLYNK_MANAGER_H

#include <Arduino.h>

void initBlynkManager();
void pushStatusToBlynk();
void serviceBlynk();
void logBlynkEvent(const char *eventCode, const String &message);
void writeBlynkString(int virtualPin, const String &value);

#endif
