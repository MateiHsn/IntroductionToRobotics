// HardwareManager.h
#ifndef HARDWARE_MANAGER_HPP
#define HARDWARE_MANAGER_HPP

#include <Arduino.h>
#include "GameModel.hpp"

// Sound Types
enum SoundType {
    SOUND_MENU_MOVE,
    SOUND_MENU_SELECT,
    SOUND_PLAYER_MOVE,
    SOUND_CUP_COLLECT,
    SOUND_PLAYER_DEATH,
    SOUND_ROOM_CLEAR,
    SOUND_VICTORY,
    SOUND_GAME_OVER,
    SOUND_NONE
};

// Melody Structure
struct MelodyNote {
    unsigned int frequency;
    unsigned int duration;
};

class HardwareManager {
private:
    // Pin References
    const byte photosensorPin;
    const byte backlightPin;
    const byte defeatLightPin;
    const byte winLightPin;
    const byte bonusLightPin;
    const byte buzzerPin;
    
    // Backlight Management
    unsigned long lastBacklightCheckTime;
    const unsigned int BACKLIGHT_CHECK_INTERVAL = 500; // Check every 500ms
    const int BRIGHTNESS_THRESHOLD = 300; // Adjust based on your photosensor
    bool backlightState;
    bool autoBacklightEnabled;
    
    // LED Blink Management
    unsigned long ledBlinkStartTime;
    unsigned int ledBlinkDuration;
    unsigned int ledBlinkInterval;
    byte ledBlinkCount;
    byte ledBlinkMaxCount;
    byte ledBlinkPin;
    bool ledBlinkActive;
    bool ledBlinkState;
    unsigned long lastLEDToggleTime;
    
    // Multi-LED Blink (for victory animation)
    bool multiLedBlinkActive;
    byte multiLedPins[3];
    byte multiLedPinCount;
    
    // Buzzer Management (Non-blocking Melody)
    bool buzzerEnabled;
    SoundType currentSound;
    const MelodyNote* currentMelody;
    byte melodyLength;
    byte currentNoteIndex;
    unsigned long noteStartTime;
    bool isMelodyPlaying;
    
    // Predefined Melodies
    static const MelodyNote cupCollectMelody[];
    static const MelodyNote playerDeathMelody[];
    static const MelodyNote roomClearMelody[];
    static const MelodyNote victoryMelody[];
    static const MelodyNote gameOverMelody[];
    
    // Helper Methods
    void turnOffAllLEDs();
    void startLEDBlink(byte pin, byte count, unsigned int interval);
    void startMultiLEDBlink(const byte* pins, byte pinCount, byte count, unsigned int interval);
    void updateLEDBlink();
    void startMelody(const MelodyNote* melody, byte length);
    void updateMelody();
    void playSimpleTone(unsigned int frequency, unsigned int duration);
    
public:
    HardwareManager(byte photoPin, byte backlightPin, byte defeatPin, 
                   byte winPin, byte bonusPin, byte buzzerPin);
    
    // Initialization
    void initialize();
    
    // Backlight Control
    void updateBacklight();
    void setAutoBacklight(bool enabled);
    bool getAutoBacklight() const;
    void setBacklight(bool on);
    
    // Status LED Control
    void updateStatusLEDs(GameState state);
    void blinkDefeatLED();
    void blinkWinLED();
    void blinkBonusLED();
    
    // Buzzer Control
    void playSound(SoundType type);
    void setBuzzerEnabled(bool enabled);
    bool getBuzzerEnabled() const;
    void updateBuzzer(); // Non-blocking buzzer management
    void stopSound();
    bool isSoundPlaying() const;
    
    // Update Method (call this in main loop)
    void update(GameState currentState);
};

#endif // HARDWARE_MANAGER_H
