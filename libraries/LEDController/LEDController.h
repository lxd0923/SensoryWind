#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <FastLED.h>

#define NUM_LEDS 12
#define LED_PIN 3
#define BRIGHTNESS 128
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define DENIED_COLOR CRGB::Red
#define DENIED_BLINK_INTERVAL 200
#define DENIED_BLINK_COUNT 3
//#define LEVEL_SHOW_TIMEOUT 3000

class LEDController {
public:
  LEDController();
  void begin();
  void setVolumeLevel(int level);       // Update LEDs and volumeLevel
  void wakeUpEffect();                  // Start wake-up animation
  void update();                        // Update current animation (called in loop)
  void waveDisplay(int pwmValue, int minPwm, int maxPwm);       // Wave mode display
  void waiting();                       // Waiting animation
  void deniedAnimation();               // Denied NFC animation
  void clear();                         // Turn off all LEDs
  bool isAnimationActive();             // Check if any animation is running
  void volumeLevelShow();
  bool isStartingUp();

private:
  enum State {
    IDLE,
    WAKE_UP,
    TRANSITION,
    WAITING,
    WAVE,
    DENIED
  };

  CRGB leds[NUM_LEDS];
  CRGBPalette16 currentPalette;
  TBlendType currentBlending;

  int volumeLevel = 1;                 // Class variable for volume level (default 1)
  State currentState = IDLE;           // Current state machine state
  float wakeUpProgress = 0.0;          // Wake-up animation progress (0.0 to 1.0)
  const unsigned long wakeUpDuration = 5000; // Total duration in ms (5s)
  const long wakeupEffectInterval = 50;       // Wake-up update interval in ms
  int transitionLED = 0;                // Number of LEDs turned off in transition
  int litLEDs = NUM_LEDS;              // Number of LEDs currently lit
  const long transitionInterval = 100;  // Transition interval per LED in ms
  uint8_t volumeBaseHue = 0;           // Base hue for setVolumeLevel and transition
  bool ledState = false;                // Waiting blink state
  const unsigned long blinkInterval = 1000;   // Waiting blink interval
  const unsigned long waveInterval = 500;     // Wave update interval
  int wavePwmValue = 0;                 // Current pwmValue for WAVE state
  uint8_t waveColorIndex = 0;           // Wave color index
  uint8_t waveFadeFactor = 0;           // Wave fade factor
  uint8_t blinkCounter = 0;             // Denied animation blink counter

  int minPwmValue = 0;
  int maxPwmValue = 255;

  void startWakeUpEffect();            // Initialize wake-up animation
  void updateWakeUpLEDs(float progress); // Update LEDs during wake-up
  void updateTransitionLEDs();          // Update LEDs during transition
  void setState(State newState);        // Change state and reset variables
};

#endif