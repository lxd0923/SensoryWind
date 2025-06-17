#ifndef ESP_DEVICE_CLIENT_H
#define ESP_DEVICE_CLIENT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Preferences.h> 
#include <esp_system.h> 
#include <cstdio> 


class ESPDeviceClient {
  public:
    /**
     * @brief Constructor for the ESPDeviceClient.
     * Initializes the client with backend details and device name.
     * @param url The base URL for sending events (e.g., Supabase Edge Function URL).
     * @param token The Bearer token for authentication with the backend.
     * @param name The human-readable name for the device.
     */
    ESPDeviceClient(const String& url, const String& token, const String& name)
      : supabaseUrl(url), bearerToken(token), deviceName(name) {}

    /**
     * @brief Initializes the ESPDeviceClient, loads the device UUID from NVS,
     * or generates and saves a new one if none exists.
     * This method MUST be called after creating the client object and
     * BEFORE calling any sendEvent methods.
     */
    void begin() {
      DEBUG_PRINTLN("ESPDeviceClient initialized");
      preferences.begin("device_prefs", false);

      deviceId = preferences.getString("uuid", "");

      if (deviceId == "") {
        DEBUG_PRINTLN("UUID not found in NVS. Generating a new one...");
        deviceId = generateUuid(); 

        preferences.putString("uuid", deviceId);
        DEBUG_PRINTLN("Generated and saved new UUID: " + deviceId);
      } else {
          DEBUG_PRINTLN("Loaded existing UUID from NVS: " + deviceId);
      }

      preferences.end();

      DEBUG_PRINTLN("ESPDeviceClient initialization complete. Device ID is ready.");
    }

    /**
     * @brief Sends a "device_on" event to the backend.
     * Typically called when the device starts up or becomes active.
     */
    void sendDeviceOnEvent() {
      sendEvent("on", 0, true);
    }

    /**
     * @brief Sends a "heartbeat" event to the backend.
     * Used to signal that the device is still alive and functioning.
     * @param uptimeSeconds The device's uptime in seconds.
     */
    void sendHeartbeatEvent(unsigned long uptimeSeconds) {
      sendEvent("heartbeat", uptimeSeconds, false);
    }

    /**
     * @brief Sends a "device_off" event to the backend.
     * Typically called before the device shuts down.
     * @param uptimeSeconds The device's total uptime before shutting down.
     */
    void sendDeviceOffEvent(unsigned long uptimeSeconds) {
      sendEvent("off", uptimeSeconds, false);
    }

    /**
     * @brief Get the generated or loaded device UUID.
     * Useful if other parts of your application need the device's UUID.
     * @return The unique device UUID as a String. Returns empty string if begin() hasn't been called.
     */
    String getDeviceId() const {
        return deviceId;
    }

  private:
    String supabaseUrl;
    String bearerToken;
    String deviceId;
    String deviceName;
    Preferences preferences;

    /**
     * @brief Generates a Version 4 (random) UUID string.
     * Uses the ESP32's hardware random number generator and formats it
     * according to the standard UUID string representation.
     * Made static as it doesn't rely on object-specific data.
     * @return A newly generated UUID string (e.g., "a1b2c3d4-e5f6-4789-9012-34567890abcd").
     */  
    static String generateUuid() {
      uint8_t randomBytes[16]; 
      char uuidBuffer[37];     

      esp_fill_random(randomBytes, 16);

      randomBytes[6] = (randomBytes[6] & 0x0F) | 0x40;
      randomBytes[8] = (randomBytes[8] & 0x3F) | 0x80;

      sprintf(uuidBuffer,
              "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
              randomBytes[0], randomBytes[1], randomBytes[2], randomBytes[3],
              randomBytes[4], randomBytes[5],
              randomBytes[6], randomBytes[7],
              randomBytes[8], randomBytes[9],
              randomBytes[10], randomBytes[11], randomBytes[12], randomBytes[13], randomBytes[14], randomBytes[15]);

      return String(uuidBuffer);
    }

    /**
     * @brief Sends an event payload to the configured backend URL via HTTP POST.
     * This is a private helper method used by the public send...Event functions.
     * @param eventType The type of event (e.g., "on", "heartbeat", "off").
     * @param uptimeDelta An integer value representing uptime or time delta relevant to the event.
     * @param used A boolean indicating a state or usage related to the event.
     */
    void sendEvent(const String& eventType, int uptimeDelta, bool used) {
      HTTPClient http;
      http.begin(supabaseUrl);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", "Bearer " + bearerToken);

      String payload = "{\"device_id\":\"" + String(deviceId) + 
                      "\",\"event\":\"" + eventType + 
                      "\",\"uptime_delta\":" + String(uptimeDelta) + 
                      ",\"name\":\"" + String(deviceName) + 
                      "\",\"used\":" + String(used ? "true" : "false") + "}";

      int response = http.POST(payload);
      Serial.printf("[ESPDeviceClient] %s -> HTTP %d\n", eventType.c_str(), response);
      if (response != 200) {
        DEBUG_PRINTLN("Response: " + http.getString());
      }

      http.end();
    }
};

#endif
