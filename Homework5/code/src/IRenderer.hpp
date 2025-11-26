// IRenderer.h
#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include <Arduino.h>
#include "GameModel.hpp"

// Abstract Renderer Interface
class IRenderer {
public:
    virtual ~IRenderer() {}
    
    // Initialization
    virtual void initialize() = 0;
    
    // Clear display
    virtual void clear() = 0;
    
    // Menu Rendering
    virtual void renderMenu(MenuOption selectedOption, const unsigned int* highscores) = 0;
    
    // Game Rendering
    virtual void renderGame(const Room& currentRoom, const Player& player, 
                           unsigned int score, byte roomNumber) = 0;
    
    // Pause Screen
    virtual void renderPause() = 0;
    
    // Game Over Screen
    virtual void renderGameOver(unsigned int finalScore, bool isNewHighscore) = 0;
    
    // Victory Screen
    virtual void renderVictory(unsigned int finalScore, bool isNewHighscore) = 0;
    
    // Status Messages
    virtual void renderRoomClear(byte roomNumber, unsigned int score) = 0;
    virtual void renderRespawnMessage(unsigned int timeRemaining) = 0;
    
    // Update/Refresh (for displays that need periodic refresh)
    virtual void update() = 0;
};

#endif // IRENDERER_H