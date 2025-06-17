#include "LEDController.h"

LEDController::LEDController() {
  currentPalette = PartyColors_p;
  currentBlending = LINEARBLEND;
  volumeLevel = 1; // Default level
  volumeBaseHue = 0; // Initialize static base hue
  litLEDs = NUM_LEDS; // All LEDs lit initially
  currentState = IDLE;
}

void LEDController::begin() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  clear();
  wakeUpEffect();
}

void LEDController::wakeUpEffect() {
  setState(WAKE_UP);
}

void LEDController::updateTransitionLEDs() {
  int ledToUnlight = NUM_LEDS - transitionLED - 1; // Start from LED 11
  if (ledToUnlight >= 0) {
    leds[ledToUnlight] = CRGB::Black;
    FastLED.show();
  }
}

void LEDController::startWakeUpEffect() {
  wakeUpProgress = 0.0;
  litLEDs = NUM_LEDS;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random8(), 255, 64);
  }
  FastLED.show();
}

void LEDController::updateWakeUpLEDs(float progress) {
  uint8_t baseHue = progress * 255;
  // Map progress to brightness (64–255) for a fade-in effect
  uint8_t brightness = 64 + (uint8_t)(progress * 191); // 64 to 255

  for (int i = 0; i < NUM_LEDS; i++) {
    // Slight hue variation across LEDs for a dynamic look
    uint8_t hue = baseHue + (i * 10) % 255;
    leds[i] = CHSV(hue, 255, brightness);
  }
  FastLED.show();
}


void LEDController::setVolumeLevel(int level) {
  volumeLevel = constrain(level, 0, NUM_LEDS / 2); 
  if (currentState == WAVE) setState(IDLE);
}

void LEDController::volumeLevelShow() {
  int numActiveLEDs = volumeLevel * 2;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < numActiveLEDs) {
      uint8_t hue =  volumeBaseHue + (i * 10) % 255;
      leds[i] = CHSV(hue, 255, 255); 
    } else {
      leds[i] = CRGB::Black;
    }
  }
  litLEDs = numActiveLEDs;
  //setState(IDLE); // Reset to IDLE
  FastLED.show();
}

void LEDController::update() {
  switch (currentState) {
    case WAKE_UP:
      EVERY_N_MILLISECONDS(wakeupEffectInterval) {
        wakeUpProgress += (float)wakeupEffectInterval / wakeUpDuration;
        if (wakeUpProgress > 1.0) wakeUpProgress = 1.0;

        updateWakeUpLEDs(wakeUpProgress);

        if (wakeUpProgress >= 1.0) {
          setState(TRANSITION);
          litLEDs = NUM_LEDS;
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV(volumeBaseHue + (i * 10) % 255, 255, 255);
          }
          FastLED.show();
        }
      }
      break;

    case TRANSITION:
      EVERY_N_MILLISECONDS(transitionInterval) {
        int targetLitLEDs = min(volumeLevel * 2, NUM_LEDS);

        if (litLEDs <= targetLitLEDs) {
          setState(IDLE);
          //setVolumeLevel(volumeLevel); // Ensure final state
          volumeLevelShow(); 
          return;
        }

        updateTransitionLEDs();

        transitionLED++;
        litLEDs--;
      }
      break;

    case WAITING:
      EVERY_N_MILLISECONDS(blinkInterval) {
        ledState = !ledState;
        fill_solid(leds, NUM_LEDS, ledState ? CRGB::White : CRGB::Black);
        FastLED.show();
      }
      break;

    case WAVE:
      EVERY_N_MILLISECONDS(waveInterval) {
        int numLitLEDs = map(wavePwmValue, minPwmValue, maxPwmValue, 1, NUM_LEDS);
        waveFadeFactor = beatsin8(2, 128, 255);
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = (i < numLitLEDs) ? ColorFromPalette(currentPalette, waveColorIndex + (i * 10), waveFadeFactor, currentBlending) : CRGB::Black;
        }
        FastLED.show();
        waveColorIndex += 3;
      }
      break;

    case DENIED:
      EVERY_N_MILLISECONDS(DENIED_BLINK_INTERVAL) {
        if (blinkCounter % 2 == 0) {
          FastLED.clear();
        } else {
          fill_solid(leds, NUM_LEDS, DENIED_COLOR);
        }
        FastLED.show();
        blinkCounter++;
        if (blinkCounter >= DENIED_BLINK_COUNT * 2) {
          setState(IDLE);
          clear();
        }
      }
      break;

    case IDLE:
      volumeLevelShow();
      // No animation, strip reflects setVolumeLevel
      break;
  }
}

/*
void LEDController::waveDisplay(int pwmValue) {
  if (currentState != IDLE && currentState != WAVE) return;

  setState(WAVE);
  int numLitLEDs = map(pwmValue, 39, 255, 1, NUM_LEDS);
  waveFadeFactor = beatsin8(2, 128, 255);

  EVERY_N_MILLISECONDS(waveInterval) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = (i < numLitLEDs) ? ColorFromPalette(currentPalette, waveColorIndex + (i * 10), waveFadeFactor, currentBlending) : CRGB::Black;
    }
    FastLED.show();
    waveColorIndex += 3;
  }
}
*/
void LEDController::waveDisplay(int pwmValue, int minPwm, int maxPwm) {
  //if (currentState != IDLE) return;
  wavePwmValue = pwmValue; // Store valid pwmValue
  minPwmValue = minPwm;
  maxPwmValue = maxPwm;
  setState(WAVE);
}


void LEDController::waiting() {
  if (currentState != IDLE) return;
  setState(WAITING);
}

void LEDController::deniedAnimation() {
  if (currentState != IDLE) return;
  setState(DENIED);
  blinkCounter = 0;
}

void LEDController::clear() {
  FastLED.clear();
  FastLED.show();
  litLEDs = 0;
}

bool LEDController::isAnimationActive() {
  return currentState != IDLE;
}

bool LEDController::isStartingUp() {
  return currentState == WAKE_UP || currentState == TRANSITION;
}

void LEDController::setState(State newState) {
  currentState = newState;

  // Reset state-specific variables
  switch (newState) {
    case WAKE_UP:
      startWakeUpEffect();
      break;
    case TRANSITION:
      transitionLED = 0;
      break;
    case WAITING:
      ledState = false;
      break;
    case WAVE:
      waveColorIndex = 0;
      waveFadeFactor = 0;
      break;
    case DENIED:
      blinkCounter = 0;
      break;
    case IDLE:
      // No specific reset needed
      break;
  }
}
