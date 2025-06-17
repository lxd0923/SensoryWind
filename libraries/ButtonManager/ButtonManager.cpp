#include "ButtonManager.h"

ButtonManager::ButtonManager() {}

void ButtonManager::begin() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT);  // Assuming external pull-ups; use INPUT_PULLUP if needed
    if (ledPins[i] != -1) {
      pinMode(ledPins[i], OUTPUT);
      digitalWrite(ledPins[i], (i == VOL_UP || i == VOL_DOWN) ? HIGH : LOW);
    }
  }
}

void ButtonManager::update() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    previousStates[i] = currentStates[i];  
    currentStates[i] = isPressed(static_cast<Button>(i));  
  }
}

bool ButtonManager::isPressed(Button button) {
  int index = static_cast<int>(button);
  if (index < 0 || index >= NUM_BUTTONS) {
    return false;
  }
  return digitalRead(buttonPins[index]) == LOW;  // Active-low buttons
}

bool ButtonManager::wasPressed(Button button) {
  int index = static_cast<int>(button);
  if (index < 0 || index >= NUM_BUTTONS) {
    return false;
  }
  return currentStates[index] && !previousStates[index];  // Rising edge
}

bool ButtonManager::wasReleased(Button button) {
  int index = static_cast<int>(button);
  if (index < 0 || index >= NUM_BUTTONS) {
    return false;
  }
  return !currentStates[index] && previousStates[index];  // Falling edge
}

void ButtonManager::setLED(Button button, bool state) {
  int index = static_cast<int>(button);
  if (index < 0 || index >= NUM_BUTTONS || ledPins[index] == -1) {
    return;
  }
  digitalWrite(ledPins[index], state ? HIGH : LOW);  // Active-high LEDs
}

void ButtonManager::updateLEDs() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (i == VOL_UP || i == VOL_DOWN) {
      bool pressed = currentStates[i];  // Use stored current state
      digitalWrite(ledPins[i], pressed ? LOW : HIGH);  // Off when pressed, on when not
    }
  }
}