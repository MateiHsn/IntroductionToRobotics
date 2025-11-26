// GameController.cpp
#include "GameController.hpp"

GameController::GameController(GameModel& gameModel, HardwareManager& hwManager,
                               byte joyXPin, byte joyYPin)
    : model(gameModel), hardware(hwManager),
      joystickXPin(joyXPin), joystickYPin(joyYPin) {
    
    lastInputTime = 0;
    playerDeathTime = 0;
    waitingForRespawn = false;
    roomClearMessageShown = false;
    roomClearTime = 0;
    lastUpdateTime = 0;
}

void GameController::initialize() {
    pinMode(joystickXPin, INPUT);
    pinMode(joystickYPin, INPUT);
    
    // Initialize hardware
    hardware.initialize();
    
    // Load highscores from EEPROM
    model.loadHighscoresFromEEPROM(0);
}

// Input Reading Methods
int GameController::readJoystickX() {
    return analogRead(joystickXPin);
}

int GameController::readJoystickY() {
    return analogRead(joystickYPin);
}

bool GameController::isJoystickLeft() {
    return readJoystickX() < inputConfig.joystickDeadzoneMin;
}

bool GameController::isJoystickRight() {
    return readJoystickX() > inputConfig.joystickThreshold;
}

bool GameController::isJoystickUp() {
    return readJoystickY() < inputConfig.joystickDeadzoneMin;
}

bool GameController::isJoystickDown() {
    return readJoystickY() > inputConfig.joystickThreshold;
}

bool GameController::canAcceptInput() {
    return (millis() - lastInputTime) >= inputConfig.debouncingDelay;
}

// State Update Methods
void GameController::updateMenuState() {
    if (!canAcceptInput()) return;
    
    // Navigate menu with joystick
    if (isJoystickUp()) {
        model.selectPreviousMenuOption();
        hardware.playSound(SOUND_MENU_MOVE);
        lastInputTime = millis();
    } else if (isJoystickDown()) {
        model.selectNextMenuOption();
        hardware.playSound(SOUND_MENU_MOVE);
        lastInputTime = millis();
    }
}

void GameController::updatePlayingState() {
    handlePlayerMovement();
    checkPlayerStatus();
    checkRoomCompletion();
    
    if (waitingForRespawn) {
        handleRespawn();
    }
}

void GameController::updatePausedState() {
    // In paused state, wait for menu navigation or resume
    // This can be extended with pause menu functionality
}

void GameController::updateGameOverState() {
    // Check if score is a new highscore
    if (model.isNewHighscore(model.getScore())) {
        model.addHighscore(model.getScore());
        model.saveHighscoresToEEPROM(0);
    }
}

void GameController::updateVictoryState() {
    // Check if score is a new highscore
    if (model.isNewHighscore(model.getScore())) {
        model.addHighscore(model.getScore());
        model.saveHighscoresToEEPROM(0);
    }
}

// Game Logic Helpers
void GameController::handlePlayerMovement() {
    if (!canAcceptInput()) return;
    if (waitingForRespawn) return;
    
    bool moved = false;
    
    if (isJoystickLeft()) {
        moved = model.movePlayer(-1, 0);
        lastInputTime = millis();
    } else if (isJoystickRight()) {
        moved = model.movePlayer(1, 0);
        lastInputTime = millis();
    } else if (isJoystickUp()) {
        moved = model.movePlayer(0, -1);
        lastInputTime = millis();
    } else if (isJoystickDown()) {
        moved = model.movePlayer(0, 1);
        lastInputTime = millis();
    }
    
    if (moved) {
        hardware.playSound(SOUND_PLAYER_MOVE);
        
        // Check if player collected a cup
        const Player& player = model.getPlayer();
        if (model.getEntityAt(player.column, player.row) == '3') {
            hardware.playSound(SOUND_CUP_COLLECT);
            hardware.blinkBonusLED();
        }
    }
}

void GameController::checkPlayerStatus() {
    const Player& player = model.getPlayer();
    
    if (!player.isAlive && !waitingForRespawn) {
        // Player just died
        waitingForRespawn = true;
        playerDeathTime = millis();
        hardware.playSound(SOUND_PLAYER_DEATH);
        hardware.blinkDefeatLED();
    }
}

void GameController::checkRoomCompletion() {
    if (model.isCurrentRoomCleared() && !roomClearMessageShown) {
        roomClearMessageShown = true;
        roomClearTime = millis();
        hardware.playSound(SOUND_ROOM_CLEAR);
        hardware.blinkWinLED();
    }
    
    // Auto-advance to next room after display time
    if (roomClearMessageShown && 
        (millis() - roomClearTime) >= ROOM_CLEAR_DISPLAY_TIME) {
        
        if (model.isGameCompleted()) {
            model.setVictory();
            hardware.playSound(SOUND_VICTORY);
        } else {
            model.advanceToNextRoom();
            roomClearMessageShown = false;
        }
    }
}

void GameController::handleRespawn() {
    if ((millis() - playerDeathTime) >= inputConfig.respawnDelay) {
        model.respawnPlayer();
        waitingForRespawn = false;
    }
}

// Main Update Loop
void GameController::update() {
    unsigned long currentTime = millis();
    
    // Throttle updates to UPDATE_INTERVAL
    if (currentTime - lastUpdateTime < UPDATE_INTERVAL) {
        return;
    }
    lastUpdateTime = currentTime;
    
    // Update hardware (backlight, LEDs, buzzer)
    hardware.update(model.getState());
    
    // Update game logic based on current state
    switch (model.getState()) {
        case MENU:
            updateMenuState();
            break;
            
        case PLAYING:
            updatePlayingState();
            break;
            
        case PAUSED:
            updatePausedState();
            break;
            
        case GAME_OVER:
            updateGameOverState();
            break;
            
        case VICTORY:
            updateVictoryState();
            break;
    }
}

// External Input Handlers (called by ISRs via volatile flags)
void GameController::handleSelectButton() {
    switch (model.getState()) {
        case MENU:
            model.confirmMenuSelection();
            hardware.playSound(SOUND_MENU_SELECT);
            break;
            
        case GAME_OVER:
        case VICTORY:
            model.resetGame();
            hardware.playSound(SOUND_MENU_SELECT);
            break;
            
        default:
            break;
    }
}

void GameController::handlePauseButton() {
    switch (model.getState()) {
        case PLAYING:
            model.setState(PAUSED);
            hardware.playSound(SOUND_MENU_SELECT);
            break;
            
        case PAUSED:
            model.setState(PLAYING);
            hardware.playSound(SOUND_MENU_SELECT);
            break;
            
        default:
            break;
    }
}

bool GameController::isWaitingForRespawn() const {
    return waitingForRespawn;
}