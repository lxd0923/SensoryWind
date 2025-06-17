#ifndef STEP_DETECTOR_H
#define STEP_DETECTOR_H

#include <Arduino.h>
#include <LIS2DH12.h>

#define FILTER_WINDOW_SIZE 5
#define PRECISION 1000
#define SAMPLE_WINDOW 500
#define MIN_STEP_INTERVAL 200
#define MAX_STEP_INTERVAL 2500
#define INTERVAL_WINDOW_SIZE 5

class StepDetector {
public:
  StepDetector(LIS2DH12* accelerometer);
  void begin();
  bool detectStep(); // Returns true if step detected
  float getBikeSpeed(); // Returns computed bike speed
  void updateBikeSpeed();
private:
  LIS2DH12* accel;
  int16_t sample_old[3] = {0, 0, 0};
  int16_t sample_new[3] = {0, 0, 0};
  int16_t xBuffer[FILTER_WINDOW_SIZE] = {0};
  int16_t yBuffer[FILTER_WINDOW_SIZE] = {0};
  int16_t zBuffer[FILTER_WINDOW_SIZE] = {0};
  int bufferIndex = 0;
  bool bufferFull = false;
  int16_t ax_max = INT16_MIN, ay_max = INT16_MIN, az_max = INT16_MIN;
  int16_t ax_min = INT16_MAX, ay_min = INT16_MAX, az_min = INT16_MAX;
  int16_t ax_max_reg, ay_max_reg, az_max_reg;
  int16_t ax_min_reg, ay_min_reg, az_min_reg;
  int sampleCount = 0;
  float ax_dynamicThreshold = 0, ay_dynamicThreshold = 0, az_dynamicThreshold = 0;
  int16_t ax_peak2peak, ay_peak2peak, az_peak2peak;
  bool isIDLE = true;
  int intervalCounter = 0;
  int stepInterval = 0;
  int intervalBuffer[INTERVAL_WINDOW_SIZE] = {0};
  int intervalBufferIndex = 0;
  int numValidIntervals = 0;
  float bikeSpeed = 0;
  const int intervalTime = 5;

  void updateLinearShiftRegister(int16_t ax, int16_t ay, int16_t az);
  void getDynamicThreshold(int16_t ax, int16_t ay, int16_t az);
  void updateIntervalBuffer(int newInterval);
  float computeAverageInterval();
  float mapIntervalToSpeed(float averageInterval);
  float mapIntervalToSpeedRPM(float averageInterval);
  int findDominantAxis();
  bool checkStepCondition(int axis);
};

#endif