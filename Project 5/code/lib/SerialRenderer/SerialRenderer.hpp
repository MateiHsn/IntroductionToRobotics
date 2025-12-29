// SerialRenderer.h
#ifndef SERIAL_RENDERER_HPP
#define SERIAL_RENDERER_HPP

#include "IRenderer.hpp"

class SerialRenderer : public IRenderer {
private:
    unsigned long lastRenderTime;
    const unsigned int RENDER_INTERVAL = 500; // Render every 500ms to avoid spam
    
    // Helper Methods
    void printSeparator();
    void printCentered(const char* text);
    void printRoomRow(const char* rowData, const Player& player, byte row);
    char getDisplayChar(char entity);
    
public:
    SerialRenderer();
    
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
};

#endif // SERIAL_RENDERER_H
