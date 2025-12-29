// GameModel.h
#ifndef GAME_MODEL_HPP
#define GAME_MODEL_HPP

#include <Arduino.h>

// Game States
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    VICTORY
};

// Menu Options
enum MenuOption {
    START_GAME,
    HIGHSCORE_1,  
    HIGHSCORE_2,  
    HIGHSCORE_3,  
    MENU_OPTIONS_COUNT
};

// Entity Types (matches the enum in main.cpp)
enum EntityType {
    EMPTY = ' ',
    PLAYER_ENTITY = 2,
    FIRE_ENTITY ,
    LADDER_ENTITY ,
    CUP_ENTITY 
};

// Player Structure
struct Player {
    byte column;
    byte row;
    bool isAlive;
};

struct HighscoreData {
    unsigned int scores[3];
    byte checksum; // For validation
};

// Room Structure (2 rows x 16 columns)
struct Room {
    char topRow[17];    // 16 chars + null terminator
    char bottomRow[17]; // 16 chars + null terminator
    byte cupsInRoom;
    byte cupsCollected;
};

class GameModel {
private:
    // Game State
    GameState currentState;
    MenuOption selectedMenuOption;
    
    // Player
    Player player;
    
    // Rooms
    static const byte TOTAL_ROOMS = 6;
    Room rooms[TOTAL_ROOMS];
    byte currentRoomIndex;
    
    // Scoring
    unsigned int score;
    unsigned long roomStartTime;
    static const byte POINTS_PER_CUP = 10;
    static const byte BASE_ROOM_CLEAR_POINTS = 50;
    
    // Highscores
    static const byte HIGHSCORE_COUNT = 3;
    unsigned int highscores[HIGHSCORE_COUNT];
    
    // Room Initialization
    void initializeRooms();
    void loadRoom(byte roomIndex, const char* topRowData, const char* bottomRowData);
    
    // Helper Methods
    byte countCupsInRoom(byte roomIndex);
    void resetPlayerToRoomStart();
    
public:
    GameModel();
    
    // State Management
    GameState getState() const;
    void setState(GameState newState);
    
    // Menu Management
    MenuOption getSelectedMenuOption() const;
    void selectNextMenuOption();
    void selectPreviousMenuOption();
    void confirmMenuSelection();
    
    // Game Initialization
    void startNewGame();
    void resetGame();
    
    // Player Management
    const Player& getPlayer() const;
    bool movePlayer(int deltaColumn, int deltaRow);
    void killPlayer();
    void respawnPlayer();
    
    // Room Management
    byte getCurrentRoomIndex() const;
    const Room& getCurrentRoom() const;
    bool isCurrentRoomCleared() const;
    void advanceToNextRoom();
    bool isGameCompleted() const;
    
    // Item Interaction
    bool collectCupAt(byte column, byte row);
    bool checkFireAt(byte column, byte row) const;
    bool checkLadderAt(byte column, byte row) const;
    char getEntityAt(byte column, byte row) const;
    
    // Scoring
    unsigned int getScore() const;
    void addScore(unsigned int points);
    void calculateRoomClearBonus();
    unsigned long getRoomStartTime() const;
    void startRoomTimer();
    
    // Highscore Management
    const unsigned int* getHighscores() const;
    bool isNewHighscore(unsigned int newScore) const;
    void addHighscore(unsigned int newScore);
    byte calculateChecksum(const unsigned int* scores) const;
    void loadHighscoresFromEEPROM(int eepromAddress);
    void saveHighscoresToEEPROM(int eepromAddress) const;
    void resetHighscores();
    
    // Victory/Defeat
    void setVictory();
    void setGameOver();
};

#endif // GAME_MODEL_H
