#include "StepDetector.h"

StepDetector::StepDetector(LIS2DH12* accelerometer) : accel(accelerometer) {}

void StepDetector::begin() {
  while (!accel->begin()) {
    Serial.println("Initialization failed, please check the connection and I2C address settings");
    delay(1000);
  }
  accel->setMode(LIS2DH12::HIGH_RESOLUTION_MODE);
  accel->setRange(LIS2DH12::eLIS2DH12_2g);
  accel->setAcquireRate(LIS2DH12::eDataRate_200Hz);
}

bool StepDetector::detectStep() {
  if (!accel->isDataAvailable()) return false;

  int16_t ax, ay, az;
  accel->getAcceleration(&ax, &ay, &az);
  updateLinearShiftRegister(ax, ay, az);
  getDynamicThreshold(ax, ay, az);

  int dominantAxis = findDominantAxis();
  bool isStep = checkStepCondition(dominantAxis);

/*
  int16_t differences[3] = {
    abs(sample_new[0] - sample_old[0]),
    abs(sample_new[1] - sample_old[1]),
    abs(sample_new[2] - sample_old[2])
  };
  int maxDiff = differences[0], axis = 0;
  if (differences[1] > maxDiff) { maxDiff = differences[1]; axis = 1; }
  if (differences[2] > maxDiff) { maxDiff = differences[2]; axis = 2; }

  bool isStep = false;
  if (axis == 0 && sample_new[0] < sample_old[0] && sample_new[0] < ax_dynamicThreshold) {
    isStep = true;
    isIDLE = false;
  }
  else if (axis == 1 && sample_new[1] < sample_old[1] && sample_new[1] < ay_dynamicThreshold) {
    isStep = true;
    isIDLE = false;
  }
  else if (axis == 2 && sample_new[2] < sample_old[2] && sample_new[2] < az_dynamicThreshold) {
    isStep = true;
    isIDLE = false;
  }
  */

  if (!isIDLE) intervalCounter++;
  stepInterval = intervalCounter * intervalTime;

  // Validate and process the step
  if (isStep) {
    if (stepInterval < MIN_STEP_INTERVAL) {
      isStep = false;
    } else {
      isStep = true; 
      
      updateIntervalBuffer(stepInterval);
      updateBikeSpeed();
      intervalCounter = 0;
    }
  }

  if (stepInterval > MAX_STEP_INTERVAL) {
    isStep = false;
    isIDLE = true;
    intervalCounter = 0;
    bikeSpeed = 0;
    memset(intervalBuffer, 0, sizeof(intervalBuffer));
    intervalBufferIndex = 0;
    numValidIntervals = 0;
  }

  return isStep;
}

void StepDetector::updateBikeSpeed() {
  // Only update speed if we have enough valid intervals
  if (numValidIntervals > 3) {
    float avgInterval = computeAverageInterval();
    bikeSpeed = mapIntervalToSpeedRPM(avgInterval);
    // Serial.print("Bike speed updated: "); Serial.print(bikeSpeed); Serial.println(" km/h");
  } else {
    // Not enough data yet; keep speed at 0 or last value
    bikeSpeed = 0.0f; // Or leave as-is, depending on preference
    // Serial.println("Not enough intervals to update bike speed");
  }
}


int StepDetector::findDominantAxis() {
  int16_t differences[3] = {
    abs(sample_new[0] - sample_old[0]),
    abs(sample_new[1] - sample_old[1]),
    abs(sample_new[2] - sample_old[2])
  };
  int maxDiff = differences[0], axis = 0;
  if (differences[1] > maxDiff) { maxDiff = differences[1]; axis = 1; }
  if (differences[2] > maxDiff) { axis = 2; }
  return axis;
}

bool StepDetector::checkStepCondition(int axis) {
  bool isStep = false;
  switch (axis) {
    case 0: if (sample_new[0] < sample_old[0] && sample_new[0] < ax_dynamicThreshold) isStep = true; break;
    case 1: if (sample_new[1] < sample_old[1] && sample_new[1] < ay_dynamicThreshold) isStep = true; break;
    case 2: if (sample_new[2] < sample_old[2] && sample_new[2] < az_dynamicThreshold) isStep = true; break;
  }
  if (isStep) isIDLE = false;
  return isStep;
}

float StepDetector::getBikeSpeed() {
  return bikeSpeed;
}

void StepDetector::updateLinearShiftRegister(int16_t ax, int16_t ay, int16_t az) {
  memcpy(sample_old, sample_new, sizeof(sample_new));
  if (abs(ax - sample_old[0]) > PRECISION) sample_new[0] = ax;
  if (abs(ay - sample_old[1]) > PRECISION) sample_new[1] = ay;
  if (abs(az - sample_old[2]) > PRECISION) sample_new[2] = az;
}

void StepDetector::getDynamicThreshold(int16_t ax, int16_t ay, int16_t az) {
    if (sampleCount == 0) {
        ax_max = ay_max = az_max = INT16_MIN;
        ax_min = ay_min = az_min = INT16_MAX;
    }
  // Update min and max values for the current window
  if (ax > ax_max) ax_max = ax;
  if (ay > ay_max) ay_max = ay;
  if (az > az_max) az_max = az;

  if (ax < ax_min) ax_min = ax;
  if (ay < ay_min) ay_min = ay;
  if (az < az_min) az_min = az;

  // Increment sample count and calculate thresholds if window is complete
  sampleCount++;
  
  if (sampleCount >= SAMPLE_WINDOW) {
    sampleCount = 0;

    // Store max and min values
    ax_max_reg = ax_max;
    ay_max_reg = ay_max;
    az_max_reg = az_max;
    ax_min_reg = ax_min;
    ay_min_reg = ay_min;
    az_min_reg = az_min;

    // Calculate dynamic thresholds as averages of max and min values
    ax_dynamicThreshold = (ax_max + ax_min) / 2.0;
    ay_dynamicThreshold = (ay_max + ay_min) / 2.0;
    az_dynamicThreshold = (az_max + az_min) / 2.0;

    // Calculate peak-to-peak differences
    ax_peak2peak = ax_max - ax_min;
    ay_peak2peak = ay_max - ay_min;
    az_peak2peak = az_max - az_min;

    // Reset min and max for the next window
    ax_max = INT16_MIN;
    ay_max = INT16_MIN;
    az_max = INT16_MIN;
    ax_min = INT16_MAX;
    ay_min = INT16_MAX;
    az_min = INT16_MAX;
  }
}


void StepDetector::updateIntervalBuffer(int newInterval) {
  intervalBuffer[intervalBufferIndex] = newInterval;
  intervalBufferIndex = (intervalBufferIndex + 1) % INTERVAL_WINDOW_SIZE;
  if (numValidIntervals < INTERVAL_WINDOW_SIZE) numValidIntervals++;
}

float StepDetector::computeAverageInterval() {
  int sum = 0;
  for (int i = 0; i < numValidIntervals; i++) sum += intervalBuffer[i];
  return numValidIntervals > 0 ? (float)sum / numValidIntervals : 0.0f;
}

float StepDetector::mapIntervalToSpeed(float averageInterval) {
  float mappedSpeed = 50.0 * (1.0 - (averageInterval - MIN_STEP_INTERVAL) / (MAX_STEP_INTERVAL - MIN_STEP_INTERVAL));
  return constrain(mappedSpeed, 0, 50);
}


float StepDetector::mapIntervalToSpeedRPM(float averageInterval) {
  float cadence = 60000.0f / averageInterval;
  
  // Distance per revolution (m) = wheel circumference * effective gear ratio
  const float distancePerRevolution = 2.1 * 1.2f; //adjustable
  
  // Speed m/min to km/h
  float speed = cadence * distancePerRevolution * 0.06f;
  
  return constrain(speed, 0.0f, 50.0f);
}

