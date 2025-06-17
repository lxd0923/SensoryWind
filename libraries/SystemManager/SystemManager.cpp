// SystemManager.cpp
#include "SystemManager.h"
//#include "ESPDeviceClient.h" // Include header as powerOff uses it


SystemManager::SystemManager()
    : _level(DEFAULT_LEVEL)
{

}


void SystemManager::begin() {
    esp_deep_sleep_enable_gpio_wakeup(1ULL << POWER_BUTTON_GPIO, ESP_GPIO_WAKEUP_GPIO_LOW);
    gpio_hold_dis((gpio_num_t)GPIO_NUM_7);

    // Check the cause of the last wakeup.
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_GPIO) {
        DEBUG_PRINTLN("Not woken by GPIO; powering off immediately...");
        powerOff();
    } else {
        DEBUG_PRINTLN("Woken by GPIO: " + String(POWER_BUTTON_GPIO));
    }

    _buttons.begin();

    DEBUG_PRINTLN("SystemManager initialized.");
}


void SystemManager::update() {
    _buttons.update(); 

    if (_buttons.wasReleased(ButtonManager::PWR_BTN)) {
        if (_isSystemActive) {
            DEBUG_PRINTLN("Detected request: Power OFF");
            if (_powerOffRequestHandler) { 
                _powerOffRequestHandler(); 
            }
        } else {
            DEBUG_PRINTLN("Detected request: Power ON");
            if (_powerOnRequestHandler) {
                _powerOnRequestHandler(); 
            }
        }
    }

    if (_buttons.wasPressed(ButtonManager::VOL_UP)) {
        if (_level < MAX_LEVEL) {
            _level++;
            DEBUG_PRINTLN("Detected event: Level UP -> " + String(_level));
            if (_levelChangeHandler) {
                _levelChangeHandler(_level);
            }
        }
    }

     if (_buttons.wasPressed(ButtonManager::VOL_DOWN)) {
         if (_level > MIN_LEVEL) {
             _level--;
             DEBUG_PRINTLN("Detected event: Level DOWN -> " + String(_level));
              if (_levelChangeHandler) {
                _levelChangeHandler(_level);
            }
         }
     }

    if (_buttons.wasReleased(ButtonManager::LOCK_BTN)) {
        _isFanLocked = !_isFanLocked;
        _buttons.setLED(ButtonManager::LOCK_BTN, _isFanLocked);
        DEBUG_PRINTLN(_isFanLocked ? "Detected event: Lock TOGGLE -> Locked" : "Detected event: Lock TOGGLE -> Unlocked");
        if (_lockToggleHandler) {
            _lockToggleHandler(_isFanLocked); 
        }
    }


    _buttons.updateLEDs();

}

// --- Implement Observer Registration Methods ---

void SystemManager::onPowerOnRequest(std::function<void()> handler) {
    _powerOnRequestHandler = handler; 
}

void SystemManager::onPowerOffRequest(std::function<void()> handler) {
    _powerOffRequestHandler = handler; 
}

void SystemManager::onLevelChange(std::function<void(int)> handler) {
     _levelChangeHandler = handler; 
}

void SystemManager::onLockToggle(std::function<void(bool)> handler) {
     _lockToggleHandler = handler; 
}

// --- Power State Methods ---

void SystemManager::powerOn() {
    //if (!_isSystemActive) {
        _isSystemActive = true;
        DEBUG_PRINTLN("Executing: System powering ON");
    //}
}


void SystemManager::powerOff() {
     //if (_isSystemActive) {
        _isSystemActive = false; 
        DEBUG_PRINTLN("Executing: System powering OFF");
        gpio_deep_sleep_hold_en();
        gpio_hold_en((gpio_num_t)GPIO_NUM_7);

        DEBUG_PRINTLN("Entering deep sleep...");
        esp_deep_sleep_start();

        // Execution stops here.
     //}
}

// --- State Getter Methods ---

bool SystemManager::isActive() const {
    return _isSystemActive;
}

int SystemManager::getLevel() const {
    return _level;
}

bool SystemManager::isLocked() const {
    return _isFanLocked;
}