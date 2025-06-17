#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

#define NUM_BUTTONS 4   // Number of buttons managed by this class

class ButtonManager {
public:
  enum Button {
    PWR_BTN = 0,   // GPIO4 - Power button (no LED)
    VOL_UP = 1,    // GPIO1 - Volume up button (LED off when pressed)
    VOL_DOWN = 2,  // GPIO5 - Volume down button (LED off when pressed)
    LOCK_BTN = 3   // GPIO6 - Lock button (LED toggles when pressed, controlled in main)
  };

private:
  const int buttonPins[NUM_BUTTONS] = {4, 1, 5, 6};     // Input pins for buttons
  const int ledPins[NUM_BUTTONS] = {-1, 10, 19, 18};     // LED pins: -1 (none), 19, 18, 9
  bool previousStates[NUM_BUTTONS] = {false};          // State from last loop
  bool currentStates[NUM_BUTTONS] = {false};           // State from current loop

public:
  ButtonManager();
  void begin();
  void update();                               // Update current and previous states
  bool isPressed(Button button);
  bool wasPressed(Button button);              // Check rising edge using stored states
  bool wasReleased(Button button);             // Check falling edge using stored states
  void setLED(Button button, bool state);
  void updateLEDs();
};

#endif