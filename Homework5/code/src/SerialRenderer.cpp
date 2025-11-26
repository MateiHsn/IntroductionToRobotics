// SerialRenderer.cpp
#include "SerialRenderer.hpp"

SerialRenderer::SerialRenderer() {
    lastRenderTime = 0;
}

void SerialRenderer::initialize() {
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    Serial.println(F("=== Serial Renderer Initialized ==="));
    delay(100); // Small delay to ensure serial is ready
}

void SerialRenderer::clear() {
    // Clear serial monitor (send multiple newlines)
    for (byte i = 0; i < 50; i++) {
        Serial.println();
    }
}

void SerialRenderer::printSeparator() {
    Serial.println(F("=================="));
}

void SerialRenderer::printCentered(const char* text) {
    byte len = strlen(text);
    byte spaces = (18 - len) / 2; // 18 chars width for centering
    
    for (byte i = 0; i < spaces; i++) {
        Serial.print(' ');
    }
    Serial.println(text);
}

char SerialRenderer::getDisplayChar(char entity) {
    switch (entity) {
        case '0': return 'P'; // Player
        case '1': return 'F'; // Fire
        case '2': return 'H'; // Ladder
        case '3': return 'C'; // Cup
        case 'P': return 'P';
        case 'F': return 'F';
        case 'H': return 'H';
        case ' ': return ' ';
        default: return entity;
    }
}

void SerialRenderer::printRoomRow(const char* rowData, const Player& player, byte row) {
    Serial.print(F("|"));
    
    for (byte col = 0; col < 16; col++) {
        if (player.column == col && player.row == row && player.isAlive) {
            Serial.print('P'); // Player
        } else {
            Serial.print(getDisplayChar(rowData[col]));
        }
    }
    
    Serial.println(F("|"));
}

// SerialRenderer.cpp - UPDATE renderMenu method

void SerialRenderer::renderMenu(MenuOption selectedOption, const unsigned int* highscores) {
    clear();
    printSeparator();
    printCentered("MAIN MENU");
    printSeparator();
    Serial.println();
    
    // Start Game Option
    if (selectedOption == START_GAME) {
        Serial.println(F("> START GAME <"));
    } else {
        Serial.println(F("  START GAME"));
    }
    
    Serial.println();
    
    // Highscore Options
    if (selectedOption == HIGHSCORE_1) {
        Serial.println(F("> HIGHSCORE #1 <"));
        Serial.print(F("  1st Place: "));
        Serial.println(highscores[0]);
    } else {
        Serial.println(F("  HIGHSCORE #1"));
    }
    
    if (selectedOption == HIGHSCORE_2) {
        Serial.println(F("> HIGHSCORE #2 <"));
        Serial.print(F("  2nd Place: "));
        Serial.println(highscores[1]);
    } else {
        Serial.println(F("  HIGHSCORE #2"));
    }
    
    if (selectedOption == HIGHSCORE_3) {
        Serial.println(F("> HIGHSCORE #3 <"));
        Serial.print(F("  3rd Place: "));
        Serial.println(highscores[2]);
    } else {
        Serial.println(F("  HIGHSCORE #3"));
    }
    
    Serial.println();
    printSeparator();
    Serial.println(F("Navigate: UP/DOWN"));
    Serial.println(F("Select: BUTTON"));
    printSeparator();
}

void SerialRenderer::renderGame(const Room& currentRoom, const Player& player, 
                                unsigned int score, byte roomNumber) {
    clear();
    printSeparator();
    
    // Header with room and score info
    Serial.print(F("Room: "));
    Serial.print(roomNumber + 1);
    Serial.print(F("/5  Score: "));
    Serial.println(score);
    
    Serial.print(F("Cups: "));
    Serial.print(currentRoom.cupsCollected);
    Serial.print(F("/"));
    Serial.println(currentRoom.cupsInRoom);
    
    printSeparator();
    
    // Render top row
    printRoomRow(currentRoom.topRow, player, 0);
    
    // Render bottom row
    printRoomRow(currentRoom.bottomRow, player, 1);
    
    printSeparator();
    
    // Legend
    Serial.println(F("P=Player H=Ladder"));
    Serial.println(F("F=Fire   C=Cup"));
    printSeparator();
}

void SerialRenderer::renderPause() {
    clear();
    printSeparator();
    printCentered("PAUSED");
    printSeparator();
    Serial.println();
    Serial.println(F("Press PAUSE to resume"));
    Serial.println();
    printSeparator();
}

void SerialRenderer::renderGameOver(unsigned int finalScore, bool isNewHighscore) {
    clear();
    printSeparator();
    printCentered("GAME OVER");
    printSeparator();
    Serial.println();
    
    Serial.print(F("Final Score: "));
    Serial.println(finalScore);
    
    if (isNewHighscore) {
        Serial.println();
        Serial.println(F("*** NEW HIGHSCORE! ***"));
    }
    
    Serial.println();
    Serial.println(F("Press SELECT for menu"));
    printSeparator();
}

void SerialRenderer::renderVictory(unsigned int finalScore, bool isNewHighscore) {
    clear();
    printSeparator();
    printCentered("VICTORY!");
    printSeparator();
    Serial.println();
    
    Serial.print(F("Final Score: "));
    Serial.println(finalScore);
    
    if (isNewHighscore) {
        Serial.println();
        Serial.println(F("*** NEW HIGHSCORE! ***"));
    }
    
    Serial.println();
    Serial.println(F("All rooms cleared!"));
    Serial.println(F("Press SELECT for menu"));
    printSeparator();
}

void SerialRenderer::renderRoomClear(byte roomNumber, unsigned int score) {
    clear();
    printSeparator();
    printCentered("ROOM CLEARED!");
    printSeparator();
    Serial.println();
    
    Serial.print(F("Room "));
    Serial.print(roomNumber + 1);
    Serial.println(F(" Complete!"));
    
    Serial.print(F("Score: "));
    Serial.println(score);
    
    Serial.println();
    Serial.println(F("Moving to next room..."));
    printSeparator();
}

void SerialRenderer::renderRespawnMessage(unsigned int timeRemaining) {
    // Don't clear, just add message
    Serial.println();
    Serial.print(F("Respawning in "));
    Serial.print(timeRemaining);
    Serial.println(F(" seconds..."));
}

void SerialRenderer::update() {
    // Serial doesn't need periodic updates
    // This method is here to satisfy the interface
}