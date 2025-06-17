#ifndef WIND_SIMULATOR_H
#define WIND_SIMULATOR_H

#include <Arduino.h>

class WindSimulator {
public:
  WindSimulator(const float* windData, int dataSize); // Pass array and size
  void update();
  float getNaturalWindSpeed();

private:
  const float* windSpeed;               // Pointer to external array
  int arraySize;                        // Runtime size of array
  int ind = 0;
  unsigned long naturalWindMillis = 0;
  float currentNaturalWindSpeed = 0.0f;
  const unsigned long UPDATE_INTERVAL = 1000;
};

#endif