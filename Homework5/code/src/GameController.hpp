// GameController.h
#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include <Arduino.h>
#include "GameModel.hpp"
#include "HardwareManager.hpp"

// Input Configuration
struct InputConfig {
    const int joystickDeadzoneMin = 400;
    const int joystickDeadzoneMax = 600;
    const int joystickThreshold = 700;
    const unsigned int debouncingDelay = 200; // ms between inputs
    const unsigned int respawnDelay = 2000; // 2 seconds respawn delay
};

class GameController {
private:
    GameModel& model;
    HardwareManager& hardware;
    InputConfig inputConfig;
    
    // Pin References
    const byte joystickXPin;
    const byte joystickYPin;
    
    // Input State
    unsigned long lastInputTime;
    unsigned long playerDeathTime;
    bool waitingForRespawn;
    
    // Room Advancement
    bool roomClearMessageShown;
    unsigned long roomClearTime;
    const unsigned int ROOM_CLEAR_DISPLAY_TIME = 2000; // 2 seconds
    
    // Game Update Timing
    unsigned long lastUpdateTime;
    const unsigned int UPDATE_INTERVAL = 50; // 50ms = 20 updates/sec
    
    // Input Handling Methods
    int readJoystickX();
    int readJoystickY();
    bool isJoystickLeft();
    bool isJoystickRight();
    bool isJoystickUp();
    bool isJoystickDown();
    bool canAcceptInput();
    
    // State-Specific Updates
    void updateMenuState();
    void updatePlayingState();
    void updatePausedState();
    void updateGameOverState();
    void updateVictoryState();
    
    // Game Logic Helpers
    void handlePlayerMovement();
    void checkPlayerStatus();
    void checkRoomCompletion();
    void handleRespawn();
    
public:
    GameController(GameModel& gameModel, HardwareManager& hwManager,
                   byte joyXPin, byte joyYPin);
    
    // Initialization
    void initialize();
    
    // Main Update Loop
    void update();
    
    // External Input Handlers (called by ISRs)
    void handleSelectButton();
    void handlePauseButton();
    
    // Getters
    bool isWaitingForRespawn() const;
};

#endif // GAME_CONTROLLER_H