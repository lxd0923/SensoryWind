#include "SystemManager.h" 
#include "ESPDeviceClient.h"

#include <WiFi.h> 
#include <Arduino.h> 

SystemManager systemManager;
ESPDeviceClient deviceClient(SUPABASE_URL, API_BEARER_TOKEN, DEFAULT_DEVICE_NAME);

static unsigned long lastHeartbeatTime = 0;

void setup() {
  Serial.begin(115200); 

  // Just to set the FAN_PWM_GPIO to LOW
  ledcAttach(FAN_PWM_GPIO, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(FAN_PWM_GPIO, 0); 

  DEBUG_PRINTLN("\n--- System Boot ---"); 
  systemManager.begin();

  DEBUG_PRINTLN("Setup complete. Registering Handlers.");

  // 1. Handler for Power ON Request event:
  systemManager.onPowerOnRequest([&]() { 
      systemManager.powerOn();
      DEBUG_PRINTLN("Observed: Power ON requested.");
      
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      // Wait for connection to WiFi
      DEBUG_PRINTLN("Connecting to " + String(WIFI_SSID));
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        DEBUG_PRINT(".");
      }
      DEBUG_PRINTLN("\nWiFi connected");

      deviceClient.begin();
      lastHeartbeatTime = millis();
      deviceClient.sendDeviceOnEvent();

  });

  // 2. Handler for Power OFF Request event:
  systemManager.onPowerOffRequest([&]() { 
      DEBUG_PRINTLN("Observed: Power OFF requested, executing deep sleep.");
      unsigned long now = millis();
      unsigned long uptimeSeconds = (now - lastHeartbeatTime) / 1000;
      deviceClient.sendDeviceOffEvent(uptimeSeconds);

      systemManager.powerOff(); 
      
  });

  // 3. Handler for Level Change event:
  systemManager.onLevelChange([&](int newLevel) { 
      DEBUG_PRINTLN("Observed: Level changed to " + String(newLevel) + ".");

  });

  // 4. Handler for Lock Toggle event:
   systemManager.onLockToggle([&](bool isLocked) {
      DEBUG_PRINTLN(isLocked ? "Observed: Lock TOGGLED to Locked." : "Observed: Lock TOGGLED to Unlocked.");

   });



}

void loop() {

  systemManager.update();

  unsigned long now = millis();
  if (now - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    unsigned long uptimeSeconds = (now - lastHeartbeatTime) / 1000;
    deviceClient.sendHeartbeatEvent(uptimeSeconds);
    lastHeartbeatTime = now;
  }
}
