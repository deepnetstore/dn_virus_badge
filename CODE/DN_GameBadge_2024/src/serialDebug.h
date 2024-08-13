#pragma once

#include <Arduino.h>

void do_nothing(...) {}

// #define USE_DEBUGGER
#ifdef USE_DEBUGGER
#define debugBegin(x) Serial.begin(x)
#define debugln(...) Serial.println(__VA_ARGS__)
#define debugf(...) Serial.printf(__VA_ARGS__)
#define debug(...) Serial.print(__VA_ARGS__)
#else
#define debugBegin(x) Serial.begin(x)
#define debugln(...) do_nothing(__VA_ARGS__)
#define debugf(...) do_nothing(__VA_ARGS__)
#define debug(...) do_nothing(__VA_ARGS__)
#endif
