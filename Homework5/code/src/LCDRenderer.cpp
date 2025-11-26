// LCDRenderer.cpp - CORRECTED VERSION

#include "LCDRenderer.hpp"

LCDRenderer::LCDRenderer(LiquidCrystal& lcdInstance) : lcd(lcdInstance) {
    lastUpdateTime = 0;
    needsFullRedraw = true;
    lastScrollTime = 0;
    scrollPosition = 0;
    isScrolling = false;
    
    // Initialize cached rows
    memset(cachedTopRow, 0, sizeof(cachedTopRow));
    memset(cachedBottomRow, 0, sizeof(cachedBottomRow));
    memset(scrollBuffer, 0, sizeof(scrollBuffer));
}

void LCDRenderer::initialize() {
    lcd.begin(16, 2);
    lcd.clear();
    needsFullRedraw = true;
}

void LCDRenderer::clear() {
    lcd.clear();
    memset(cachedTopRow, 0, sizeof(cachedTopRow));
    memset(cachedBottomRow, 0, sizeof(cachedBottomRow));
    needsFullRedraw = true;
}

void LCDRenderer::printAt(byte col, byte row, const char* text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}

void LCDRenderer::printAt(byte col, byte row, char c) {
    lcd.setCursor(col, row);
    lcd.write(c);
}

void LCDRenderer::clearRow(byte row) {
    lcd.setCursor(0, row);
    lcd.print(F("                ")); // 16 spaces
}

void LCDRenderer::renderCenteredText(const char* text, byte row) {
    byte len = strlen(text);
    byte startCol = (16 - len) / 2;
    
    clearRow(row);
    if (len <= 16) {
        printAt(startCol, row, text);
    } else {
        // Text too long, start scrolling
        startScrollText(text);
    }
}

char LCDRenderer::convertEntityToChar(char entity) {
    switch (entity) {
        case 'P': return (char)PLAYER_ENTITY;
        case 'F': return (char)FIRE_ENTITY;
        case 'H': return (char)LADDER_ENTITY;
        case '3': return (char)CUP_ENTITY;
        case '0': return (char)PLAYER_ENTITY;
        case '1': return (char)FIRE_ENTITY;
        case '2': return (char)LADDER_ENTITY;
        case ' ': return ' ';
        default: return entity;
    }
}

bool LCDRenderer::rowNeedsUpdate(const char* newRow, const char* cachedRow) {
    return strcmp(newRow, cachedRow) != 0;
}

void LCDRenderer::renderRoomRow(const char* rowData, byte row, const Player& player, byte playerRow) {
    char displayRow[17];
    
    // Build the display row - ALWAYS include all entities and the player
    for (byte col = 0; col < 16; col++) {
        // Check if player is at this position
        if (player.column == col && player.row == playerRow && player.isAlive) {
            displayRow[col] = (char)PLAYER_ENTITY;
        } else {
            // Convert entity character to display character
            displayRow[col] = convertEntityToChar(rowData[col]);
        }
    }
    displayRow[16] = '\0';
    
    // Always update if this is the player's row OR if content changed
    char* cachedRow = (row == 0) ? cachedTopRow : cachedBottomRow;
    bool isPlayerRow = (playerRow == row);
    
    if (needsFullRedraw || isPlayerRow || rowNeedsUpdate(displayRow, cachedRow)) {
        // Clear the entire row first to prevent ghosting
        lcd.setCursor(0, row);
        
        // Write each character individually to ensure proper display
        for (byte col = 0; col < 16; col++) {
            lcd.write(displayRow[col]);
        }
        
        // Update cache
        strcpy(cachedRow, displayRow);
    }
}

void LCDRenderer::startScrollText(const char* text) {
    strncpy(scrollBuffer, text, sizeof(scrollBuffer) - 1);
    scrollBuffer[sizeof(scrollBuffer) - 1] = '\0';
    scrollPosition = 0;
    isScrolling = true;
    lastScrollTime = millis();
}

void LCDRenderer::updateScrollText() {
    if (!isScrolling) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastScrollTime < SCROLL_INTERVAL) return;
    
    byte textLen = strlen(scrollBuffer);
    if (textLen <= 16) {
        isScrolling = false;
        return;
    }
    
    clearRow(1);
    
    // Display 16 characters starting from scrollPosition
    lcd.setCursor(0, 1);
    for (byte i = 0; i < 16; i++) {
        byte pos = (scrollPosition + i) % textLen;
        lcd.write(scrollBuffer[pos]);
    }
    
    scrollPosition++;
    if (scrollPosition >= textLen) {
        scrollPosition = 0;
    }
    
    lastScrollTime = currentTime;
}

void LCDRenderer::renderMenu(MenuOption selectedOption, const unsigned int* highscores) {
    clear();
    
    if (selectedOption == START_GAME) {
        renderCenteredText("MAIN MENU", 0);
        renderCenteredText("> START GAME <", 1);
    } 
    else if (selectedOption == HIGHSCORE_1) {
        renderCenteredText("HIGHSCORES", 0);
        
        char scoreText[17];
        snprintf(scoreText, sizeof(scoreText), "1st: %u", highscores[0]);
        renderCenteredText(scoreText, 1);
    }
    else if (selectedOption == HIGHSCORE_2) {
        renderCenteredText("HIGHSCORES", 0);
        
        char scoreText[17];
        snprintf(scoreText, sizeof(scoreText), "2nd: %u", highscores[1]);
        renderCenteredText(scoreText, 1);
    }
    else if (selectedOption == HIGHSCORE_3) {
        renderCenteredText("HIGHSCORES", 0);
        
        char scoreText[17];
        snprintf(scoreText, sizeof(scoreText), "3rd: %u", highscores[2]);
        renderCenteredText(scoreText, 1);
    }
    
    needsFullRedraw = false;
}

void LCDRenderer::renderGame(const Room& currentRoom, const Player& player, 
                             unsigned int score, byte roomNumber) {
    isScrolling = false; // Stop any scrolling
    
    // CRITICAL FIX: Clear both cached rows to force full redraw
    // This prevents ghosting from previous frames
    memset(cachedTopRow, 0, sizeof(cachedTopRow));
    memset(cachedBottomRow, 0, sizeof(cachedBottomRow));
    
    // Render top row (row 0 on LCD)
    renderRoomRow(currentRoom.topRow, 0, player, 0);
    
    // Render bottom row (row 1 on LCD)
    renderRoomRow(currentRoom.bottomRow, 1, player, 1);
    
    needsFullRedraw = false;
}

void LCDRenderer::renderPause() {
    clear();
    renderCenteredText("PAUSED", 0);
    renderCenteredText("Press to resume", 1);
    needsFullRedraw = false;
}

void LCDRenderer::renderGameOver(unsigned int finalScore, bool isNewHighscore) {
    clear();
    
    renderCenteredText("GAME OVER", 0);
    
    char scoreText[17];
    if (isNewHighscore) {
        snprintf(scoreText, sizeof(scoreText), "NEW HI: %u", finalScore);
    } else {
        snprintf(scoreText, sizeof(scoreText), "Score: %u", finalScore);
    }
    renderCenteredText(scoreText, 1);
    
    needsFullRedraw = false;
}

void LCDRenderer::renderVictory(unsigned int finalScore, bool isNewHighscore) {
    clear();
    
    renderCenteredText("VICTORY!", 0);
    
    char scoreText[17];
    if (isNewHighscore) {
        snprintf(scoreText, sizeof(scoreText), "NEW HI: %u", finalScore);
    } else {
        snprintf(scoreText, sizeof(scoreText), "Score: %u", finalScore);
    }
    renderCenteredText(scoreText, 1);
    
    needsFullRedraw = false;
}

void LCDRenderer::renderRoomClear(byte roomNumber, unsigned int score) {
    clear();
    
    renderCenteredText("ROOM CLEARED!", 0);
    
    char scoreText[17];
    snprintf(scoreText, sizeof(scoreText), "Score: %u", score);
    renderCenteredText(scoreText, 1);
    
    needsFullRedraw = false;
}

void LCDRenderer::renderRespawnMessage(unsigned int timeRemaining) {
    // Show respawn countdown on bottom row
    char message[17];
    snprintf(message, sizeof(message), "Respawn in %u", timeRemaining);
    
    clearRow(0);
    clearRow(1);
    renderCenteredText(message, 1);
}

void LCDRenderer::update() {
    unsigned long currentTime = millis();
    
    // Update scrolling text if active
    if (isScrolling) {
        updateScrollText();
    }
    
    lastUpdateTime = currentTime;
}

void LCDRenderer::forceRedraw() {
    needsFullRedraw = true;
    // Also clear cache to ensure everything redraws
    memset(cachedTopRow, 0, sizeof(cachedTopRow));
    memset(cachedBottomRow, 0, sizeof(cachedBottomRow));
}