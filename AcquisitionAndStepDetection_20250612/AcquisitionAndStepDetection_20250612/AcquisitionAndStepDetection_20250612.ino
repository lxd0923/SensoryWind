#include "SystemManager.h"
#include "StepDetector.h"
#include "WindSimulator.h"
#include "ESPDeviceClient.h"
#include "LEDController.h"
#include <esp_task_wdt.h>

#define WDT_TIMEOUT_SECONDS 30


static const float myWindSpeeds[] = {
    0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.76, 3.89, 6.05, 6.15, 5.11, 4.79, 3.46,
    2.09, 1.22, 1.55, 1.19, 1.08, 1.40, 2.99, 3.42, 3.82, 3.06, 2.70, 3.13, 5.36,
    6.19, 6.15, 4.46, 3.78, 2.99, 2.99, 2.30, 1.87, 1.73, 1.51, 1.26, 1.12, 1.30,
    1.19, 0.94, 0.68, 0.68, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
    0.00, 0.00, 2.23, 4.50, 4.46, 4.07, 4.03, 3.46, 3.02, 4.79, 6.84, 5.00, 4.10,
    5.15, 6.60, 4.03, 4.54, 5.40, 4.75, 3.78, 3.35, 3.56, 4.67, 7.67, 7.05, 6.05,
    4.75, 4.36, 3.89, 3.89, 4.18, 3.17, 2.77, 2.48, 4.10, 4.97, 5.11, 3.56, 1.55,
    1.66, 2.27, 2.92, 3.06, 2.84, 2.88, 5.25, 8.21, 8.96, 8.24, 7.82, 6.05, 4.10,
    4.10, 6.05, 6.73, 6.12, 6.05, 5.59, 5.72, 5.72, 5.43, 6.41, 5.40, 4.97, 4.46,
    4.61, 4.39, 4.21, 3.63, 3.27, 2.74, 2.41, 2.23, 2.09, 1.73, 1.40, 1.19, 0.97,
    0.76, 0.65, 0.68, 2.74, 5.22, 4.97, 5.50, 5.58, 6.15, 6.15, 5.73, 5.43, 4.86,
    3.74, 3.17, 2.70, 2.16, 2.23, 3.06, 4.10, 3.74, 2.77, 2.38, 1.84, 1.44, 1.22,
    1.04, 0.68, 0.68, 2.02, 2.70, 5.65, 5.98, 5.08, 4.86, 4.97, 4.82, 4.75, 5.54,
    5.54, 4.07, 2.92, 2.45, 2.16, 1.87, 3.56, 4.82, 5.40, 4.64, 3.31, 2.77, 2.23,
    1.84, 1.66, 1.69, 1.87, 2.30, 2.30, 2.59, 5.15, 5.15, 4.61, 4.39, 4.94, 5.11,
    7.16, 7.19, 6.91, 5.43, 4.54, 3.49, 3.74, 4.61, 4.46, 4.94, 5.40, 6.05, 6.47,
    6.30, 6.26, 4.39, 3.56, 2.92, 4.75, 5.08, 4.68, 5.15, 5.23, 4.82, 5.25, 4.86,
    4.39, 3.31, 3.02, 2.77, 2.38, 1.87, 1.80, 1.73, 3.02, 6.30, 6.05, 5.65, 4.31,
    3.49, 2.59, 2.16, 1.80, 2.12, 5.36, 6.26, 6.34, 6.44, 5.83, 4.97, 3.92, 3.35,
    3.49, 5.54, 4.64, 6.34, 9.12, 8.03
};

struct PwmLevel {
  int minPwm;
  int maxPwm;
};

const PwmLevel PWM_LEVELS[6] = {
  {0,   105},  // L1: 0–105 
  {30,  135},  // L2: 30–135 
  {60,  165},  // L3: 60–165 
  {90,  195},  // L4: 90–195 
  {120, 225},  // L5: 120–225 
  {150, 255},  // L6: 150–255 
};

SystemManager systemManager;
LEDController ledController;
const int WIND_SIZE = sizeof(myWindSpeeds) / sizeof(myWindSpeeds[0]);
LIS2DH12 accel(&Wire, LIS2DH12_ADDR); 
StepDetector stepDetector(&accel);    
WindSimulator windSim(myWindSpeeds, WIND_SIZE);

ESPDeviceClient deviceClient(SUPABASE_URL, API_BEARER_TOKEN, DEFAULT_DEVICE_NAME);

static unsigned long lastHeartbeatTime = 0;

long unsigned lastActiveTime = 0;
const unsigned long WIND_SPEED_UPDATE_TIME = 1000;
const unsigned long LEVEL_SHOW_TIMEOUT = 3000;

enum WiFiState {
    WIFI_IDLE,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_FAILED 
};

WiFiState currentWiFiState = WIFI_IDLE;
unsigned long wifiConnectAttemptStartTime = 0;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 60000;


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Just to set the FAN_PWM_GPIO to LOW
  ledcAttach(FAN_PWM_GPIO, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(FAN_PWM_GPIO, 0);

  Serial.println("=== System Manager Debug Start ===");

  systemManager.begin();
  stepDetector.begin();
  ledController.begin();

  DEBUG_PRINTLN("Setup complete. Registering Handlers.");


  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = WDT_TIMEOUT_SECONDS * 1000, // Timeout in milliseconds
      .trigger_panic = true,                    // Trigger panic on timeout
  };
  //esp_task_wdt_init(&twdt_config);
  //esp_task_wdt_add(NULL);

  // 1. Handler for Power ON Request event:
  systemManager.onPowerOnRequest([&]() { 
      systemManager.powerOn();
      DEBUG_PRINTLN("Observed: Power ON requested.");
      if (currentWiFiState == WIFI_IDLE || currentWiFiState == WIFI_FAILED) {
          currentWiFiState = WIFI_CONNECTING; 
          WiFi.mode(WIFI_STA); 
          WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
          wifiConnectAttemptStartTime = millis(); 
          DEBUG_PRINTLN("Starting WiFi connection...");
      }
  });

  // 2. Handler for Power OFF Request event:
  systemManager.onPowerOffRequest([&]() { 
      DEBUG_PRINTLN("Observed: Power OFF requested, executing deep sleep.");
      
      if (currentWiFiState == WIFI_CONNECTED) {
          unsigned long now = millis();
          unsigned long uptimeSeconds = (now - lastHeartbeatTime) / 1000;
          deviceClient.sendDeviceOffEvent(uptimeSeconds);
          WiFi.disconnect(true); // Disconnect and delete credentials
          currentWiFiState = WIFI_IDLE;
        }
      
      ledController.clear();
      systemManager.powerOff(); 
      
  });

  // 3. Handler for Level Change event:
  systemManager.onLevelChange([&](int newLevel) { 
      ledController.setVolumeLevel(newLevel);
      lastActiveTime = millis();
      DEBUG_PRINTLN("Observed: Level changed to " + String(newLevel) + ".");
  });

  // 4. Handler for Lock Toggle event:
   systemManager.onLockToggle([&](bool isLocked) {
      DEBUG_PRINTLN(isLocked ? "Observed: Lock TOGGLED to Locked." : "Observed: Lock TOGGLED to Unlocked.");
   });

ledController.setVolumeLevel(systemManager.getLevel());

}

void loop() {
  //esp_task_wdt_reset();

  switch (currentWiFiState) {
      case WIFI_IDLE:
          // Do nothing, waiting for a power ON request to start connecting
          break;

      case WIFI_CONNECTING:
          if (WiFi.status() == WL_CONNECTED) {
              DEBUG_PRINTLN("\nWiFi connected!");
              deviceClient.begin(); // Initialize your device client after connection
              lastHeartbeatTime = millis(); // Reset heartbeat timer
              deviceClient.sendDeviceOnEvent(); // Send device on event
              currentWiFiState = WIFI_CONNECTED;
          } else if (millis() - wifiConnectAttemptStartTime > WIFI_CONNECT_TIMEOUT_MS) {
              DEBUG_PRINTLN("\nWiFi connection timed out!");
              WiFi.disconnect(true); // Attempt to disconnect and clear credentials
              currentWiFiState = WIFI_FAILED; // Go to a failed state
              DEBUG_PRINTLN("Retrying WiFi connection on next power ON request.");
          } else {
               static unsigned long lastWifiStatusPrint = 0;
              if (millis() - lastWifiStatusPrint > 1000) { // Print dot every second
                   DEBUG_PRINTLN("Connecting");
                  lastWifiStatusPrint = millis();
              }
           }
           break;

      case WIFI_CONNECTED:
           // Wi-Fi is connected, handle heartbeats
           if (millis() - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
              unsigned long uptimeSeconds = (millis() - lastHeartbeatTime) / 1000;
              deviceClient.sendHeartbeatEvent(uptimeSeconds);
              lastHeartbeatTime = millis();
          }
          break;

      case WIFI_FAILED:
          // Wi-Fi connection failed, might try again on next power ON or after a cool-down
          // For now, it stays in this state until a new power ON event triggers it.
          break;
  }

  systemManager.update();
  ledController.update();
  windSim.update();

  bool stepDetected = stepDetector.detectStep();

  if (ledController.isStartingUp()) lastActiveTime = millis();

  if (stepDetected) {
    Serial.println("Step detected!");

  }
  static unsigned long lastPrintMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastPrintMillis >= WIND_SPEED_UPDATE_TIME) {
    if (!systemManager.isLocked()) {
      float bikeSpeed = stepDetector.getBikeSpeed();
      float naturalWindSpeed = windSim.getNaturalWindSpeed();
      float windSpeed = bikeSpeed + naturalWindSpeed;
      if (windSpeed > 50.0) {
        windSpeed = 50.0;
      }
      int pwmValue = map(windSpeed, 0, 50, PWM_LEVELS[systemManager.getLevel()-1].minPwm, PWM_LEVELS[systemManager.getLevel()-1].maxPwm);

      
      if (currentMillis - lastActiveTime >= LEVEL_SHOW_TIMEOUT) {
        ledController.waveDisplay(pwmValue, PWM_LEVELS[systemManager.getLevel()-1].minPwm,PWM_LEVELS[systemManager.getLevel()-1].maxPwm);
      }
    // Debug output
      Serial.print("Bike Speed: ");
      Serial.print(bikeSpeed);
      Serial.print(" km/h | Natural Wind: ");
      Serial.print(naturalWindSpeed);
      Serial.print(" km/h | Combined Wind: ");
      Serial.print(windSpeed);
      Serial.print(" km/h | PWM: ");
      Serial.println(pwmValue);
      
    // Update 
      ledcWrite(FAN_PWM_GPIO, pwmValue);
    } else {
      Serial.println("System is locked. PWM not updated.");
    }
    lastPrintMillis = currentMillis;
  }

  /*
  if (currentMillis - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    unsigned long uptimeSeconds = (currentMillis - lastHeartbeatTime) / 1000;
    deviceClient.sendHeartbeatEvent(uptimeSeconds);
    lastHeartbeatTime = currentMillis;
  }
  */

}
