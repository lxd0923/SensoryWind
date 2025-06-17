#ifndef CONFIG_H 
#define CONFIG_H 

#include <Arduino.h>

// --- Hardware Pin Definitions ---
#define POWER_BUTTON_GPIO       4   // GPIO connected to the power/wakeup button (must be an RTC GPIO for deep sleep wakeup)
#define FAN_PWM_GPIO            7   // GPIO used for Fan PWM control
#define VOLUME_UP_GPIO          1   // GPIO for Volume Up button
#define VOLUME_DOWN_GPIO        5   // GPIO for Volume Down button
#define LOCK_BUTTON_GPIO        6   // GPIO for Lock button

#define PWM_CHANNEL 0  
#define PWM_FREQ 25000 
#define PWM_RESOLUTION 8 

// --- System Levels / Limits ---
const int MIN_LEVEL = 1;        // Minimum system level 
const int MAX_LEVEL = 6;        // Maximum system level 
const int DEFAULT_LEVEL = 3;    // Default starting level

// --- Timing Constants (in milliseconds unless specified) ---
const unsigned long HEARTBEAT_INTERVAL_MS = 60000; // Send heartbeat every 60 seconds

// --- Network / Backend Configuration ---
static const char* WIFI_SSID = "Hyperoptic Fibre A723";        // <<<<<<<<<<< CHANGE THIS
static const char* WIFI_PASSWORD = "4CeFudQ33QjpjJ";    // <<<<<<<<<<< CHANGE THIS
static const char* SUPABASE_URL = "https://rlcfammvzjnrwwnjjkds.supabase.co/functions/v1/handle-device-event"; 
static const char* API_BEARER_TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InJsY2ZhbW12empucnd3bmpqa2RzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MzgyNzU1NDUsImV4cCI6MjA1Mzg1MTU0NX0.v5UByrvXPSEvPX1QsRhJkXAjIYex_gGQaAcIlTrv8LM"; 
static const char* DEFAULT_DEVICE_NAME = "Fan A"; // Default name 

// --- Partition Related Constants ---
static const char* NVS_PREFERENCES_NAMESPACE = "device_prefs"; 


// --- Debugging Configuration ---
#define DEBUG_ENABLED // <<<<<<<<<<< COMMENT OUT THIS LINE TO DISABLE DEBUG PRINTS

/**/
#ifdef DEBUG_ENABLED 
  #define DEBUG_PRINT(x, format...) Serial.print(x, ##format) 
  #define DEBUG_PRINTLN(x, format...) Serial.println(x, ##format) 
#else 
  #define DEBUG_PRINT(x, format...) do {} while (0) 
  #define DEBUG_PRINTLN(x, format...) do {} while (0) 
#endif

#endif 
