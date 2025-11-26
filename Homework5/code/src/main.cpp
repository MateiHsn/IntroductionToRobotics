#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#include "GameModel.hpp"
#include "HardwareManager.hpp"
#include "GameController.hpp"
#include "IRenderer.hpp"
#include "LCDRenderer.hpp"
#include "SerialRenderer.hpp"

// LCD Pins
const byte RS_LCD_PIN = 8;
const byte EN_LCD_PIN = 9;
const byte D4_LCD_PIN = 4;
const byte D5_LCD_PIN = 5;
const byte D6_LCD_PIN = 6;
const byte D7_LCD_PIN = 7;

// Input Pins
const byte JOYSTICK_X_AXIS_PIN = A0;
const byte JOYSTICK_Y_AXIS_PIN = A1;
const byte JOYSTICK_BUTTON_PIN = 2; // Interrupt pin
const byte PAUSE_BUTTON_PIN = 3;    // Interrupt pin

// Sensor Pins
const byte PHOTOSENSOR_PIN = A2;

// Output Pins
const byte BACKLIGHT_PIN = 10;
const byte DEFEAT_LIGHT_PIN = A3;
const byte WIN_LIGHT_PIN = A4;
const byte BONUS_LIGHT_PIN = A5;
const byte BUZZER_PIN = 11;

// Choose renderer: true = LCD, false = Serial (for debugging)
const bool USE_LCD_RENDERER = true;

// EEPROM address for highscores
const int EEPROM_ADDRESS = 0;

// Debouncing
const unsigned int DEBOUNCING_TIME = 200; // milliseconds

// Rendering timing
const unsigned int RENDER_INTERVAL = 200; // Render every 200ms

// Hardware
LiquidCrystal lcd(RS_LCD_PIN, EN_LCD_PIN, D4_LCD_PIN, D5_LCD_PIN, D6_LCD_PIN, D7_LCD_PIN);

// Game System
GameModel gameModel;
HardwareManager hardwareManager(PHOTOSENSOR_PIN, BACKLIGHT_PIN, DEFEAT_LIGHT_PIN, 
                                WIN_LIGHT_PIN, BONUS_LIGHT_PIN, BUZZER_PIN);
GameController gameController(gameModel, hardwareManager, 
                              JOYSTICK_X_AXIS_PIN, JOYSTICK_Y_AXIS_PIN);

// Renderers (only one will be used based on USE_LCD_RENDERER)
LCDRenderer lcdRenderer(lcd);
SerialRenderer serialRenderer;
IRenderer* activeRenderer = nullptr; // Pointer to active renderer

// Timing
unsigned long lastRenderTime = 0;
unsigned long lastDebugTime = 0;

volatile bool selectButtonPressed = false;
volatile bool pauseButtonPressed = false;
volatile unsigned long lastButtonPressTime = 0;

void selectButtonISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPressTime > DEBOUNCING_TIME) {
        selectButtonPressed = true;
        lastButtonPressTime = currentTime;
    }
}

void pauseButtonISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPressTime > DEBOUNCING_TIME) {
        pauseButtonPressed = true;
        lastButtonPressTime = currentTime;
    }
}

byte playerCharacter[] = {
    B01110,
    B01110,
    B01110,
    B00100,
    B11111,
    B00100,
    B01010,
    B01010
};

byte fireCharacter[] = {
    B00000,
    B00110,
    B01110,
    B01100,
    B11110,
    B11110,
    B11111,
    B01110
};

byte ladderCharacter[] = {
    B10001,
    B11111,
    B11111,
    B10001,
    B10001,
    B11111,
    B11111,
    B10001
};

byte cupCharacter[] = {
    B10001,
    B11111,
    B11111,
    B01110,
    B01110,
    B00100,
    B01110,
    B11111
};

void renderCurrentState() {
    unsigned long currentTime = millis();
    
    // Throttle rendering to prevent excessive updates
    if (currentTime - lastRenderTime < RENDER_INTERVAL) {
        return;
    }
    lastRenderTime = currentTime;
    
    GameState currentState = gameModel.getState();
    
    switch (currentState) {
        case MENU:
            activeRenderer->renderMenu(gameModel.getSelectedMenuOption(), 
                                      gameModel.getHighscores());
            break;
            
        case PLAYING:
            // Check if waiting for respawn
            if (gameController.isWaitingForRespawn()) {
                // Calculate remaining respawn time
                // This is handled internally by controller, just show message
                activeRenderer->renderRespawnMessage(2);
            } else if (gameModel.isCurrentRoomCleared()) {
                // Room cleared message
                activeRenderer->renderRoomClear(gameModel.getCurrentRoomIndex(), 
                                               gameModel.getScore());
            } else {
                // Normal gameplay
                activeRenderer->renderGame(gameModel.getCurrentRoom(), 
                                          gameModel.getPlayer(),
                                          gameModel.getScore(), 
                                          gameModel.getCurrentRoomIndex());
            }
            break;
            
        case PAUSED:
            activeRenderer->renderPause();
            break;
            
        case GAME_OVER:
            activeRenderer->renderGameOver(gameModel.getScore(), 
                                          gameModel.isNewHighscore(gameModel.getScore()));
            break;
            
        case VICTORY:
            activeRenderer->renderVictory(gameModel.getScore(), 
                                         gameModel.isNewHighscore(gameModel.getScore()));
            break;
    }
    
    // Update renderer (for animations, scrolling, etc.)
    activeRenderer->update();
}

void printDebugInfo() {
    unsigned long currentTime = millis();
    const unsigned long DEBUG_INTERVAL = 2000; // Print every 2 seconds
    
    if (currentTime - lastDebugTime < DEBUG_INTERVAL) {
        return;
    }
    lastDebugTime = currentTime;
    
    Serial.println(F("\n=== DEBUG INFO ==="));
    Serial.print(F("State: "));
    
    switch (gameModel.getState()) {
        case MENU: Serial.println(F("MENU")); break;
        case PLAYING: Serial.println(F("PLAYING")); break;
        case PAUSED: Serial.println(F("PAUSED")); break;
        case GAME_OVER: Serial.println(F("GAME_OVER")); break;
        case VICTORY: Serial.println(F("VICTORY")); break;
    }
    
    Serial.print(F("Room: "));
    Serial.print(gameModel.getCurrentRoomIndex() + 1);
    Serial.print(F("/5  Score: "));
    Serial.println(gameModel.getScore());
    
    const Player& player = gameModel.getPlayer();
    Serial.print(F("Player: ("));
    Serial.print(player.column);
    Serial.print(F(", "));
    Serial.print(player.row);
    Serial.print(F(") Alive: "));
    Serial.println(player.isAlive ? F("Yes") : F("No"));
    
    const Room& room = gameModel.getCurrentRoom();
    Serial.print(F("Cups: "));
    Serial.print(room.cupsCollected);
    Serial.print(F("/"));
    Serial.println(room.cupsInRoom);
    
    Serial.println(F("==================\n"));
}

void handleSerialCommands() {
    if (!Serial.available()) {
        return;
    }
    
    char cmd = Serial.read();
    
    switch (cmd) {
        case 'r': // Reset highscores
            gameModel.resetHighscores();
            gameModel.saveHighscoresToEEPROM(EEPROM_ADDRESS);
            Serial.println(F("Highscores reset!"));
            break;
            
        case 'b': // Toggle buzzer
            hardwareManager.setBuzzerEnabled(!hardwareManager.getBuzzerEnabled());
            Serial.print(F("Buzzer: "));
            Serial.println(hardwareManager.getBuzzerEnabled() ? F("ON") : F("OFF"));
            break;
            
        case 'a': // Toggle auto backlight
            hardwareManager.setAutoBacklight(!hardwareManager.getAutoBacklight());
            Serial.print(F("Auto backlight: "));
            Serial.println(hardwareManager.getAutoBacklight() ? F("ON") : F("OFF"));
            break;
            
        case 'l': // Manual backlight toggle
            hardwareManager.setAutoBacklight(false);
            hardwareManager.setBacklight(!hardwareManager.getAutoBacklight());
            Serial.println(F("Backlight toggled"));
            break;
            
        case 'd': // Toggle debug output
            Serial.println(F("Debug info will appear every 2 seconds"));
            break;
            
        case 'h': // Help
            Serial.println(F("\n=== COMMANDS ==="));
            Serial.println(F("r - Reset highscores"));
            Serial.println(F("b - Toggle buzzer"));
            Serial.println(F("a - Toggle auto backlight"));
            Serial.println(F("l - Manual backlight toggle"));
            Serial.println(F("d - Show debug info"));
            Serial.println(F("h - Show this help"));
            Serial.println(F("================\n"));
            break;
            
        default:
            break;
    }
}

void setup() {
    // Initialize Serial (always useful for debugging)
    Serial.begin(9600);
    
    // Wait for Serial to be ready (non-blocking approach)
    unsigned long serialStartTime = millis();
    while (!Serial && (millis() - serialStartTime < 1000)) {
        // Wait up to 1 second for serial, but don't block indefinitely
    }
    
    Serial.println(F("=== Prince of Persia-like Game Starting ==="));
    
    // Setup input pins
    pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(JOYSTICK_X_AXIS_PIN, INPUT);
    pinMode(JOYSTICK_Y_AXIS_PIN, INPUT);
    
    // Setup interrupts
    attachInterrupt(digitalPinToInterrupt(JOYSTICK_BUTTON_PIN), selectButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PAUSE_BUTTON_PIN), pauseButtonISR, FALLING);
    
    // Initialize LCD and create custom characters
    if (USE_LCD_RENDERER) {
        lcd.begin(16, 2);
        lcd.createChar(EntityType::PLAYER_ENTITY, playerCharacter);
        lcd.createChar(EntityType::FIRE_ENTITY, fireCharacter);
        lcd.createChar(EntityType::LADDER_ENTITY, ladderCharacter);
        lcd.createChar(EntityType::CUP_ENTITY, cupCharacter);
        
        activeRenderer = &lcdRenderer;
        Serial.println(F("Using LCD Renderer"));
    } else {
        activeRenderer = &serialRenderer;
        Serial.println(F("Using Serial Renderer"));
    }
    
    // Initialize renderer
    activeRenderer->initialize();
    
    // Initialize game controller (which initializes hardware and loads highscores)
    gameController.initialize();
    
    // Initial render
    activeRenderer->clear();
    renderCurrentState();
    
    Serial.println(F("Setup complete! Game ready."));
    Serial.println(F("Type 'h' for help commands"));
}

void loop() {
    // Process interrupt flags
    if (selectButtonPressed) {
        selectButtonPressed = false;
        gameController.handleSelectButton();
    }
    
    if (pauseButtonPressed) {
        pauseButtonPressed = false;
        gameController.handlePauseButton();
    }
    
    // Handle serial commands (optional debugging)
    handleSerialCommands();
    
    // Update game controller (handles input, game logic, hardware)
    gameController.update();
    
    // Render current state (throttled to RENDER_INTERVAL)
    renderCurrentState();
    
    // Optional: Print debug info (throttled to 2 seconds)
    // Uncomment the line below to enable debug output
    // printDebugInfo();
}
