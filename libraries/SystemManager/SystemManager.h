#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>       
#include <driver/gpio.h>   
#include "ButtonManager.h" 
#include "config.h"       

#include <functional>      

class SystemManager {
public:

    SystemManager();
    /**
     * @brief Performs initial setup for the system after boot or wakeup.
     * Configures deep sleep wakeup sources (GPIOs), checks the cause of the last reset,
     * and initializes the ButtonManager. This method should be called once in setup().
     */
    void begin();
    /**
     * @brief Updates button states and detects system events (like button presses/releases).
     * This function should be called frequently in the main loop().
     * It calls ButtonManager::update() and checks for specific button events.
     * When events are detected, it calls any registered handler functions for that event.
     */
    void update(); 

    /**
     * @brief Sets the system state to active (on).
     * This method changes the internal '_isSystemActive' flag and performs any
     * actions needed when the system transitions to the ON state. 
     */
    void powerOn(); 

    /**
     * @brief Initiates the system power-off sequence and enters deep sleep.
     * Called by an observer callback function registered for the OFF request event.
     */
    void powerOff(); 

    // --- State Getter Methods ---
    bool isActive() const;
    int getLevel() const;
    bool isLocked() const;

    // --- Observer Registration Methods (Public Event Points) ---
    /**
     * @brief Register a handler function to be called when a Power ON request is detected.
     * @param handler The function to call (signature: void()). Use std::function or a lambda.
     */
    void onPowerOnRequest(std::function<void()> handler);

    /**
     * @brief Register a handler function to be called when a Power OFF request is detected.
     * @param handler The function to call (signature: void()). Use std::function or a lambda.
     */
    void onPowerOffRequest(std::function<void()> handler);

    /**
     * @brief Register a handler function to be called when a Level Change event occurs.
     * @param handler The function to call (signature: void(int newLevel)). Use std::function or a lambda.
     */
    void onLevelChange(std::function<void(int newLevel)> handler);

    /**
     * @brief Register a handler function to be called when a Lock Toggle event occurs.
     * @param handler The function to call (signature: void(bool isLocked)). Use std::function or a lambda.
     */
    void onLockToggle(std::function<void(bool isLocked)> handler);


private:
    // --- Internal State Variables ---
    bool _isSystemActive = false;
    int _level = DEFAULT_LEVEL;
    bool _isFanLocked = false;

    ButtonManager _buttons; // Instance of the ButtonManager

    // --- Storage for Registered Handlers (Observers) ---
    // These std::function objects store the callbacks provided by observers.
    std::function<void()> _powerOnRequestHandler;   // Handler for ON requests
    std::function<void()> _powerOffRequestHandler;  // Handler for OFF requests
    std::function<void(int)> _levelChangeHandler;   // Handler for level changes
    std::function<void(bool)> _lockToggleHandler;   // Handler for lock toggles

    // Note: ESPDeviceClient is NOT a member variable here.
};

#endif // SYSTEM_MANAGER_H

