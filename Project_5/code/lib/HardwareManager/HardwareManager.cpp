// HardwareManager.cpp
#include "HardwareManager.hpp"

// Define Static Melodies
const MelodyNote HardwareManager::cupCollectMelody[] = {
    {1000, 80}, {1200, 80}, {1400, 80}
};

const MelodyNote HardwareManager::playerDeathMelody[] = {
    {800, 100}, {600, 100}, {400, 100}, {200, 200}
};

const MelodyNote HardwareManager::roomClearMelody[] = {
    {800, 100}, {1000, 100}, {1200, 100}, {1400, 100}, {1600, 200}
};

const MelodyNote HardwareManager::victoryMelody[] = {
    {523, 150}, {659, 150}, {784, 150}, {1047, 400}, 
    {784, 150}, {1047, 150}, {1318, 500}
};

const MelodyNote HardwareManager::gameOverMelody[] = {
    {400, 200}, {350, 200}, {300, 200}, {250, 400}
};

HardwareManager::HardwareManager(byte photoPin, byte backlightPin, byte defeatPin, 
                                 byte winPin, byte bonusPin, byte buzzerPin)
    : photosensorPin(photoPin), backlightPin(backlightPin), 
      defeatLightPin(defeatPin), winLightPin(winPin), 
      bonusLightPin(bonusPin), buzzerPin(buzzerPin) {
    
    lastBacklightCheckTime = 0;
    backlightState = true;
    autoBacklightEnabled = true;
    
    ledBlinkStartTime = 0;
    ledBlinkDuration = 0;
    ledBlinkInterval = 0;
    ledBlinkCount = 0;
    ledBlinkMaxCount = 0;
    ledBlinkPin = 0;
    ledBlinkActive = false;
    ledBlinkState = false;
    lastLEDToggleTime = 0;
    
    multiLedBlinkActive = false;
    multiLedPinCount = 0;
    
    buzzerEnabled = true;
    currentSound = SOUND_NONE;
    currentMelody = nullptr;
    melodyLength = 0;
    currentNoteIndex = 0;
    noteStartTime = 0;
    isMelodyPlaying = false;
}

void HardwareManager::initialize() {
    pinMode(photosensorPin, INPUT);
    pinMode(backlightPin, OUTPUT);
    pinMode(defeatLightPin, OUTPUT);
    pinMode(winLightPin, OUTPUT);
    pinMode(bonusLightPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    
    // Initial states
    digitalWrite(backlightPin, HIGH);
    turnOffAllLEDs();
    noTone(buzzerPin);
}

// Backlight Control
void HardwareManager::updateBacklight() {
    if (!autoBacklightEnabled) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastBacklightCheckTime >= BACKLIGHT_CHECK_INTERVAL) {
        int brightness = analogRead(photosensorPin);
        
        // If it's dark, turn backlight ON; if bright, turn OFF
        bool shouldBeOn = (brightness < BRIGHTNESS_THRESHOLD);
        
        if (shouldBeOn != backlightState) {
            backlightState = shouldBeOn;
            digitalWrite(backlightPin, backlightState ? HIGH : LOW);
        }
        
        lastBacklightCheckTime = currentTime;
    }
}

void HardwareManager::setAutoBacklight(bool enabled) {
    autoBacklightEnabled = enabled;
    if (!enabled) {
        // If auto is disabled, ensure backlight is on
        setBacklight(true);
    }
}

bool HardwareManager::getAutoBacklight() const {
    return autoBacklightEnabled;
}

void HardwareManager::setBacklight(bool on) {
    backlightState = on;
    digitalWrite(backlightPin, on ? HIGH : LOW);
}

// LED Management Helper Methods
void HardwareManager::turnOffAllLEDs() {
    digitalWrite(defeatLightPin, LOW);
    digitalWrite(winLightPin, LOW);
    digitalWrite(bonusLightPin, LOW);
}

void HardwareManager::startLEDBlink(byte pin, byte count, unsigned int interval) {
    ledBlinkPin = pin;
    ledBlinkMaxCount = count;
    ledBlinkCount = 0;
    ledBlinkInterval = interval;
    ledBlinkActive = true;
    ledBlinkState = false;
    ledBlinkStartTime = millis();
    lastLEDToggleTime = millis();
    
    multiLedBlinkActive = false; // Disable multi-LED if single LED starts
}

void HardwareManager::startMultiLEDBlink(const byte* pins, byte pinCount, byte count, unsigned int interval) {
    multiLedBlinkActive = true;
    multiLedPinCount = pinCount;
    
    for (byte i = 0; i < pinCount && i < 3; i++) {
        multiLedPins[i] = pins[i];
    }
    
    ledBlinkMaxCount = count;
    ledBlinkCount = 0;
    ledBlinkInterval = interval;
    ledBlinkState = false;
    ledBlinkStartTime = millis();
    lastLEDToggleTime = millis();
    
    ledBlinkActive = false; // Disable single LED if multi-LED starts
}

void HardwareManager::updateLEDBlink() {
    if (!ledBlinkActive && !multiLedBlinkActive) return;
    
    unsigned long currentTime = millis();
    
    // Check if we've completed all blinks
    if (ledBlinkCount >= ledBlinkMaxCount * 2) { // *2 because each blink = ON + OFF
        if (ledBlinkActive) {
            digitalWrite(ledBlinkPin, LOW);
            ledBlinkActive = false;
        }
        if (multiLedBlinkActive) {
            for (byte i = 0; i < multiLedPinCount; i++) {
                digitalWrite(multiLedPins[i], LOW);
            }
            multiLedBlinkActive = false;
        }
        return;
    }
    
    // Toggle LEDs at interval
    if (currentTime - lastLEDToggleTime >= ledBlinkInterval) {
        ledBlinkState = !ledBlinkState;
        
        if (ledBlinkActive) {
            digitalWrite(ledBlinkPin, ledBlinkState ? HIGH : LOW);
        }
        
        if (multiLedBlinkActive) {
            for (byte i = 0; i < multiLedPinCount; i++) {
                digitalWrite(multiLedPins[i], ledBlinkState ? HIGH : LOW);
            }
        }
        
        ledBlinkCount++;
        lastLEDToggleTime = currentTime;
    }
}

// Status LED Control
void HardwareManager::updateStatusLEDs(GameState state) {
    // Update any active blink animations
    updateLEDBlink();
    
    // Don't override active blink animations
    if (ledBlinkActive || multiLedBlinkActive) return;
    
    unsigned long currentTime = millis();
    static unsigned long lastStateLEDUpdate = 0;
    static bool stateLEDState = false;
    const unsigned int STATE_LED_BLINK_INTERVAL = 500;
    
    switch (state) {
        case MENU:
            turnOffAllLEDs();
            break;
            
        case PLAYING:
            turnOffAllLEDs();
            digitalWrite(bonusLightPin, HIGH); // Indicates game is active
            break;
            
        case PAUSED:
            // Blink bonus LED to indicate pause
            if (currentTime - lastStateLEDUpdate >= STATE_LED_BLINK_INTERVAL) {
                stateLEDState = !stateLEDState;
                digitalWrite(bonusLightPin, stateLEDState ? HIGH : LOW);
                lastStateLEDUpdate = currentTime;
            }
            break;
            
        case GAME_OVER:
            turnOffAllLEDs();
            digitalWrite(defeatLightPin, HIGH);
            break;
            
        case VICTORY:
            turnOffAllLEDs();
            digitalWrite(winLightPin, HIGH);
            break;
    }
}

void HardwareManager::blinkDefeatLED() {
    startLEDBlink(defeatLightPin, 6, 100); // 6 blinks, 100ms interval
}

void HardwareManager::blinkWinLED() {
    byte pins[] = {winLightPin, bonusLightPin};
    startMultiLEDBlink(pins, 2, 3, 200); // 3 blinks, 200ms interval
}

void HardwareManager::blinkBonusLED() {
    startLEDBlink(bonusLightPin, 2, 100); // 2 quick blinks, 100ms interval
}

// Buzzer Control
void HardwareManager::playSimpleTone(unsigned int frequency, unsigned int duration) {
    if (!buzzerEnabled) return;
    
    tone(buzzerPin, frequency);
    noteStartTime = millis();
    isMelodyPlaying = true;
    currentNoteIndex = 0;
    melodyLength = 1;
    
    // Create a temporary single-note melody
    static MelodyNote tempNote;
    tempNote.frequency = frequency;
    tempNote.duration = duration;
    currentMelody = &tempNote;
}

void HardwareManager::startMelody(const MelodyNote* melody, byte length) {
    if (!buzzerEnabled) return;
    
    currentMelody = melody;
    melodyLength = length;
    currentNoteIndex = 0;
    isMelodyPlaying = true;
    
    // Start first note
    if (length > 0) {
        tone(buzzerPin, melody[0].frequency);
        noteStartTime = millis();
    }
}

void HardwareManager::updateMelody() {
    if (!isMelodyPlaying || currentMelody == nullptr) return;
    
    unsigned long currentTime = millis();
    unsigned long noteDuration = currentMelody[currentNoteIndex].duration;
    
    // Check if current note has finished
    if (currentTime - noteStartTime >= noteDuration) {
        noTone(buzzerPin);
        
        // Small pause between notes (50ms)
        if (currentTime - noteStartTime >= noteDuration + 50) {
            currentNoteIndex++;
            
            // Check if melody is complete
            if (currentNoteIndex >= melodyLength) {
                isMelodyPlaying = false;
                currentMelody = nullptr;
                currentSound = SOUND_NONE;
                noTone(buzzerPin);
                return;
            }
            
            // Start next note
            tone(buzzerPin, currentMelody[currentNoteIndex].frequency);
            noteStartTime = millis();
        }
    }
}

void HardwareManager::playSound(SoundType type) {
    if (!buzzerEnabled) return;
    if (type == currentSound && isMelodyPlaying) return; // Already playing this sound
    
    currentSound = type;
    
    switch (type) {
        case SOUND_MENU_MOVE:
            playSimpleTone(800, 50);
            break;
            
        case SOUND_MENU_SELECT:
            playSimpleTone(1200, 100);
            break;
            
        case SOUND_PLAYER_MOVE:
            playSimpleTone(600, 30);
            break;
            
        case SOUND_CUP_COLLECT:
            startMelody(cupCollectMelody, 3);
            break;
            
        case SOUND_PLAYER_DEATH:
            startMelody(playerDeathMelody, 4);
            break;
            
        case SOUND_ROOM_CLEAR:
            startMelody(roomClearMelody, 5);
            break;
            
        case SOUND_VICTORY:
            startMelody(victoryMelody, 7);
            break;
            
        case SOUND_GAME_OVER:
            startMelody(gameOverMelody, 4);
            break;
            
        case SOUND_NONE:
            stopSound();
            break;
    }
}

void HardwareManager::setBuzzerEnabled(bool enabled) {
    buzzerEnabled = enabled;
    if (!enabled) {
        stopSound();
    }
}

bool HardwareManager::getBuzzerEnabled() const {
    return buzzerEnabled;
}

void HardwareManager::updateBuzzer() {
    updateMelody();
}

void HardwareManager::stopSound() {
    noTone(buzzerPin);
    isMelodyPlaying = false;
    currentMelody = nullptr;
    currentSound = SOUND_NONE;
}

bool HardwareManager::isSoundPlaying() const {
    return isMelodyPlaying;
}

void HardwareManager::update(GameState currentState) {
    updateBacklight();
    updateStatusLEDs(currentState);
    updateBuzzer();
}
