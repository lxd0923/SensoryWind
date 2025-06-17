#include "WindSimulator.h"

WindSimulator::WindSimulator(const float* windData, int dataSize) {
  windSpeed = (windData && dataSize > 0) ? windData : nullptr;
  arraySize = (windData && dataSize > 0) ? dataSize : 0;
  naturalWindMillis = millis();
  currentNaturalWindSpeed = arraySize > 0 ? windSpeed[0] : 0.0f;
}

void WindSimulator::update() {
  if (!windSpeed || arraySize == 0) return;
  
  unsigned long currentMillis = millis();
  if (currentMillis - naturalWindMillis >= UPDATE_INTERVAL) {
    naturalWindMillis = currentMillis;
    currentNaturalWindSpeed = windSpeed[ind];
    ind = (ind + 1) % arraySize;
  }
}

float WindSimulator::getNaturalWindSpeed() {
  return currentNaturalWindSpeed;
}