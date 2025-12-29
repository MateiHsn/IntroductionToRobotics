// GameModel.cpp
#include "GameModel.hpp"
#include <EEPROM.h>

GameModel::GameModel() {
    currentState = MENU;
    selectedMenuOption = START_GAME;
    currentRoomIndex = 0;
    score = 0;
    roomStartTime = 0;
    
    // Initialize highscores to 0
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        highscores[i] = 0;
    }
    
    // Initialize player
    player.column = 0;
    player.row = 1;
    player.isAlive = true;
    
    // Initialize rooms with predefined layouts
    initializeRooms();
}

void GameModel::initializeRooms() {
    // Room 0 - Tutorial room (simple)
    // NOTE: 'P' will be removed automatically by resetPlayerToRoomStart()
    loadRoom(0, 
        "   3    H      3", 
        "P       H       ");
    
    // Room 1 - Fire introduction
    loadRoom(1, 
        "  3    H   3F   ", 
        "P      H    F   ");
    
    // Room 2 - More complex
    loadRoom(2, 
        "3     H FF H   3", 
        "P     H    H3 FF");
    
    // Room 3 - Challenge room
    loadRoom(3, 
        "3  H     H F H 3", 
        "P  H  F  H   H 3");
    
    // Room 4 - Final room
    loadRoom(4, 
        "3 H   3 F H    3", 
        "P H       H 3FF ");
    loadRoom(5,
        "3 H 3 3 H F H 3 ",
        "P H F F H 3 H  3");
}

void GameModel::loadRoom(byte roomIndex, const char* topRowData, const char* bottomRowData) {
    if (roomIndex >= TOTAL_ROOMS) return;
    
    // Copy room data
    strncpy(rooms[roomIndex].topRow, topRowData, 16);
    rooms[roomIndex].topRow[16] = ' ';
    
    strncpy(rooms[roomIndex].bottomRow, bottomRowData, 16);
    rooms[roomIndex].bottomRow[16] = ' ';
    
    // Count cups in this room
    rooms[roomIndex].cupsInRoom = countCupsInRoom(roomIndex);
    rooms[roomIndex].cupsCollected = 0;
}


byte GameModel::countCupsInRoom(byte roomIndex) {
    if (roomIndex >= TOTAL_ROOMS) return 0;
    
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        if (rooms[roomIndex].topRow[i] == '3') count++;
        if (rooms[roomIndex].bottomRow[i] == '3') count++;
    }
    return count;
}

void GameModel::resetPlayerToRoomStart() {
    // Find 'P' in current room or default to (0, 1)
    bool found = false;
    
    for (byte col = 0; col < 16 && !found; col++) {
        if (rooms[currentRoomIndex].topRow[col] == 'P') {
            player.column = col;
            player.row = 0;
            rooms[currentRoomIndex].topRow[col] = ' '; // REMOVE 'P' from map
            found = true;
        } else if (rooms[currentRoomIndex].bottomRow[col] == 'P') {
            player.column = col;
            player.row = 1;
            rooms[currentRoomIndex].bottomRow[col] = ' '; // REMOVE 'P' from map
            found = true;
        }
    }
    
    if (!found) {
        // Default starting position if no 'P' found
        player.column = 0;
        player.row = 1;
    }
    
    player.isAlive = true;
}

// State Management
GameState GameModel::getState() const {
    return currentState;
}

void GameModel::setState(GameState newState) {
    currentState = newState;
}

// Menu Management
MenuOption GameModel::getSelectedMenuOption() const {
    return selectedMenuOption;
}

void GameModel::selectNextMenuOption() {
    selectedMenuOption = (MenuOption)((selectedMenuOption + 1) % MENU_OPTIONS_COUNT);
}

void GameModel::selectPreviousMenuOption() {
    if (selectedMenuOption == 0) {
        selectedMenuOption = (MenuOption)(MENU_OPTIONS_COUNT - 1);
    } else {
        selectedMenuOption = (MenuOption)(selectedMenuOption - 1);
    }
}

void GameModel::confirmMenuSelection() {
    if (selectedMenuOption == START_GAME) {
        startNewGame();
    }
    // HIGHSCORE option just displays, no state change needed
}

// Game Initialization
void GameModel::startNewGame() {
    currentState = PLAYING;
    currentRoomIndex = 0;
    score = 0;
    
    // Reset all rooms
    for (byte i = 0; i < TOTAL_ROOMS; i++) {
        rooms[i].cupsCollected = 0;
    }
    
    resetPlayerToRoomStart();
    startRoomTimer();
}

void GameModel::resetGame() {
    currentState = MENU;
    selectedMenuOption = START_GAME;
    currentRoomIndex = 0;
    score = 0;
    roomStartTime = 0;
}

// Player Management
const Player& GameModel::getPlayer() const {
    return player;
}

bool GameModel::movePlayer(int deltaColumn, int deltaRow) {
    if (!player.isAlive) return false;
    
    int newColumn = player.column + deltaColumn;
    int newRow = player.row + deltaRow;
    
    // Boundary check
    if (newColumn < 0 || newColumn >= 16 || newRow < 0 || newRow >= 2) {
        return false;
    }
    
    // Check if moving vertically (requires ladder)
    if (deltaRow != 0) {
        if (!checkLadderAt(player.column, player.row)) {
            return false;
        }
    }
    
    // Check destination
    char destinationEntity = getEntityAt(newColumn, newRow);
    
    // Can't move through solid objects (none in this simple version)
    // Update player position
    player.column = newColumn;
    player.row = newRow;
    
    // Check for fire collision
    if (checkFireAt(player.column, player.row)) {
        killPlayer();
    }
    
    // Check for cup collection
    if (destinationEntity == '3') {
        collectCupAt(player.column, player.row);
    }
    
    return true;
}

void GameModel::killPlayer() {
    player.isAlive = false;
}

void GameModel::respawnPlayer() {
    resetPlayerToRoomStart();
}

// Room Management
byte GameModel::getCurrentRoomIndex() const {
    return currentRoomIndex;
}

const Room& GameModel::getCurrentRoom() const {
    return rooms[currentRoomIndex];
}

bool GameModel::isCurrentRoomCleared() const {
    return rooms[currentRoomIndex].cupsCollected >= rooms[currentRoomIndex].cupsInRoom;
}

void GameModel::advanceToNextRoom() {
    if (currentRoomIndex < TOTAL_ROOMS - 1) {
        calculateRoomClearBonus();
        currentRoomIndex++;
        resetPlayerToRoomStart();
        startRoomTimer();
    } else {
        setVictory();
    }
}

bool GameModel::isGameCompleted() const {
    return currentRoomIndex >= TOTAL_ROOMS - 1 && isCurrentRoomCleared();
}

// Item Interaction
bool GameModel::collectCupAt(byte column, byte row) {
    if (column >= 16 || row >= 2) return false;
    
    char* rowData = (row == 0) ? rooms[currentRoomIndex].topRow : rooms[currentRoomIndex].bottomRow;
    
    if (rowData[column] == '3') {
        rowData[column] = ' ';
        rooms[currentRoomIndex].cupsCollected++;
        addScore(POINTS_PER_CUP);
        return true;
    }
    return false;
}

bool GameModel::checkFireAt(byte column, byte row) const {
    if (column >= 16 || row >= 2) return false;
    
    const char* rowData = (row == 0) ? rooms[currentRoomIndex].topRow : rooms[currentRoomIndex].bottomRow;
    return (rowData[column] == 'F' || rowData[column] == '1');
}

bool GameModel::checkLadderAt(byte column, byte row) const {
    if (column >= 16 || row >= 2) return false;
    
    const char* rowData = (row == 0) ? rooms[currentRoomIndex].topRow : rooms[currentRoomIndex].bottomRow;
    return (rowData[column] == 'H' || rowData[column] == '2');
}

char GameModel::getEntityAt(byte column, byte row) const {
    if (column >= 16 || row >= 2) return ' ';
    
    const char* rowData = (row == 0) ? rooms[currentRoomIndex].topRow : rooms[currentRoomIndex].bottomRow;
    return rowData[column];
}

// Scoring
unsigned int GameModel::getScore() const {
    return score;
}

void GameModel::addScore(unsigned int points) {
    score += points;
}

void GameModel::calculateRoomClearBonus() {
    unsigned long elapsedTime = (millis() - roomStartTime) / 1000; // Convert to seconds
    
    if (elapsedTime == 0) elapsedTime = 1; // Avoid division by zero
    
    unsigned int bonus = BASE_ROOM_CLEAR_POINTS - elapsedTime;
    if (bonus < 1) bonus = 1; // Minimum bonus
    
    addScore(bonus);
}

unsigned long GameModel::getRoomStartTime() const {
    return roomStartTime;
}

void GameModel::startRoomTimer() {
    roomStartTime = millis();
}

// Highscore Management
const unsigned int* GameModel::getHighscores() const {
    return highscores;
}

bool GameModel::isNewHighscore(unsigned int newScore) const {
    return newScore > highscores[HIGHSCORE_COUNT - 1];
}

void GameModel::addHighscore(unsigned int newScore) {
    // Find the position where the new score should be inserted
    int insertPosition = -1;
    
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        if (newScore > highscores[i]) {
            insertPosition = i;
            break;
        }
        if(newScore == highscores[i]){
          break;
        }
    }
    
    if (insertPosition != -1) {
        for (byte j = HIGHSCORE_COUNT - 1; j > insertPosition; j--) {
            highscores[j] = highscores[j - 1];
        }
        highscores[insertPosition] = newScore;
    }

}

byte GameModel::calculateChecksum(const unsigned int* scores) const {
    byte checksum = 0;
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        checksum ^= (scores[i] & 0xFF);
        checksum ^= ((scores[i] >> 8) & 0xFF);
    }
    return checksum;
}

void GameModel::loadHighscoresFromEEPROM(int eepromAddress) {
    HighscoreData data;
    EEPROM.get(eepromAddress, data);
    
    // Validate with checksum
    byte calculatedChecksum = calculateChecksum(data.scores);
    
    if (calculatedChecksum == data.checksum) {
        // Data is valid, load it
        for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
            highscores[i] = data.scores[i];
            
            // Still validate for 0xFFFF (uninitialized)
            if (highscores[i] == 0xFFFF) {
                highscores[i] = 0;
            }
        }
        Serial.println(F("Highscores loaded successfully"));
    } else {
        // Corrupted or uninitialized data, reset to 0
        Serial.println(F("EEPROM data invalid, resetting highscores"));
        for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
            highscores[i] = 0;
        }
        // Save the zeroed scores
        saveHighscoresToEEPROM(eepromAddress);
    }
}

void GameModel::saveHighscoresToEEPROM(int eepromAddress) const {
    HighscoreData data;
    
    // Copy scores to struct
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        data.scores[i] = highscores[i];
    }
    
    // Calculate and store checksum
    data.checksum = calculateChecksum(data.scores);
    
    // Write to EEPROM
    EEPROM.put(eepromAddress, data);
    
    Serial.println(F("Highscores saved to EEPROM"));
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        Serial.print(F("  "));
        Serial.print(i + 1);
        Serial.print(F(": "));
        Serial.println(highscores[i]);
    }
}

void GameModel::resetHighscores() {
    for (byte i = 0; i < HIGHSCORE_COUNT; i++) {
        highscores[i] = 0;
    }
}

// Victory/Defeat
void GameModel::setVictory() {
    currentState = VICTORY;
}

void GameModel::setGameOver() {
    currentState = GAME_OVER;
}