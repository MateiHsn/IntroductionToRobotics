// LCDRenderer.h
#ifndef LCD_RENDERER_H
#define LCD_RENDERER_H

#include "IRenderer.hpp"
#include <LiquidCrystal.h>

class LCDRenderer : public IRenderer {
private:
    LiquidCrystal& lcd;
    
    // Display refresh management
    unsigned long lastUpdateTime;
    const unsigned int UPDATE_INTERVAL = 100; // Update every 100ms
    
    // Cached display state (to avoid unnecessary writes)
    char cachedTopRow[17];
    char cachedBottomRow[17];
    bool needsFullRedraw;
    
    // Scrolling text for long messages
    unsigned long lastScrollTime;
    const unsigned int SCROLL_INTERVAL = 300;
    byte scrollPosition;
    char scrollBuffer[32];
    bool isScrolling;
    
    // Helper Methods
    void printAt(byte col, byte row, const char* text);
    void printAt(byte col, byte row, char c);
    void clearRow(byte row);
    void renderRoomRow(const char* rowData, byte row, const Player& player, byte playerRow);
    char convertEntityToChar(char entity);
    void startScrollText(const char* text);
    void updateScrollText();
    void renderCenteredText(const char* text, byte row);
    bool rowNeedsUpdate(const char* newRow, const char* cachedRow);
    
public:
    LCDRenderer(LiquidCrystal& lcdInstance);
    
    // IRenderer Interface Implementation
    void initialize() override;
    void clear() override;
    void renderMenu(MenuOption selectedOption, const unsigned int* highscores) override;
    void renderGame(const Room& currentRoom, const Player& player, 
                   unsigned int score, byte roomNumber) override;
    void renderPause() override;
    void renderGameOver(unsigned int finalScore, bool isNewHighscore) override;
    void renderVictory(unsigned int finalScore, bool isNewHighscore) override;
    void renderRoomClear(byte roomNumber, unsigned int score) override;
    void renderRespawnMessage(unsigned int timeRemaining) override;
    void update() override;
    
    // Additional LCD-specific methods
    void forceRedraw();
};

#endif // LCD_RENDERER_H
