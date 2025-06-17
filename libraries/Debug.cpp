#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>  // For Serial

#define DEBUG  // Comment this line to disable debug output

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) do {} while (0)
  #define DEBUG_PRINTLN(x) do {} while (0)
#endif

#endif // DEBUG_H