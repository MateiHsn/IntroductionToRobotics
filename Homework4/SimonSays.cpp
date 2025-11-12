// Simon Says Memory Game with 4-digit 7-segment display
// Hardware: 74HC595, 4-digit 7-seg display, Joystick, Buzzer, Pause button
// Uses SPI for communication with 74HC595

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>

// Pin definitions
const int JOYSTICK_BUTTON_PIN = 2;
const int PUSHBUTTON_PIN = 3;
const int BUZZER_PIN = 9;
const int DIGIT1_PIN = 5;
const int DIGIT2_PIN = 6;
const int DIGIT3_PIN = 7;
const int DIGIT4_PIN = 8;
const int JOYSTICK_X_PIN = A0;
const int JOYSTICK_Y_PIN = A1;
const int CS_PIN = 10;          // Latch pin (STCP/RCLK)
const int COPI_PIN = 11;        // MOSI pin (DS/SER) - hardware SPI
const int SCK_PIN = 13;         // Clock pin (SHCP/SRCLK) - hardware SPI

// Display configuration
const int displayDigitsNumber = 4;
int displayDigits[] = {DIGIT1_PIN, DIGIT2_PIN, DIGIT3_PIN, DIGIT4_PIN};
const bool COMMON_CATHODE = true;  // Set based on your display type

// Character sets
const int charSetSize = 19;
const char charSet[] = {'A', 'b', 'c', 'd', 'E', 'F', 'G', 'H', 'I', 'J', 'L', 'n', 'O', 'P', 'r', 'S', 't', 'u', 'Y'};

const int numberSetSize = 10;
const char numberSet[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/**
 * Bit order for encoding:
 * DP, G, F, E, D, C, B, A
 */
const byte charSegmentEncoding[] = {
  0b01110111, // A
  0b01111100, // b
  0b01011000, // c
  0b01011110, // d
  0b01111001, // E
  0b01110001, // F
  0b00111101, // G
  0b01110110, // H
  0b00000110, // I
  0b00011110, // J
  0b00111000, // L
  0b01010100, // n
  0b00111111, // O
  0b01110011, // P
  0b01010000, // r
  0b01101101, // S
  0b01111000, // t
  0b00011100, // u
  0b01101110  // Y
};

const byte numberSegmentEncoding[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

// Text constants
const char textPlay[] = "PLAY";
const char textScore[] = "ScOr";
const char textStop[] = "StOP";
const char textPause[] = "PAuS";
const char textError[] = "Err ";

// Game states
enum gameState {
  STATE_MENU,
  STATE_SHOW_SEQUENCE,
  STATE_INPUT_PHASE,
  STATE_CHECK_ANSWER,
  STATE_RESULT,
  STATE_PAUSE,
  STATE_SHOW_SCORE,
  STATE_SHOW_PAUSE_TEXT  // New state for displaying "PAuS"
};

// Menu items
enum menuItem {
  MENU_PLAY,
  MENU_SCORE,
  MENU_STOP
};

// Character type enum
enum alphaOrNumber {
  ALPHA,
  NUMBER
};

// Game state variables
gameState currentState = STATE_MENU;
menuItem currentMenuItem = MENU_PLAY;
int currentRound = 0;
int highScore = 0;

// Game sequence and input
char gameSequence[4];
char playerInput[4];
int cursorPosition = 0;
bool digitLocked[4] = {false, false, false, false};
int selectedDigitIndex = -1;

// Timing variables
unsigned long displayStartTime = 0;
unsigned long blinkTimer = 0;
unsigned long joystickDebounceTime = 200;
unsigned long buttonPressTime = 0;
bool joystickButtonPressed = false;
bool joystickButtonLongPressed = false;
bool blinkState = false;

// Pause button interrupt variables
volatile bool pauseButtonPressed = false;
volatile unsigned long lastPauseInterruptTime = 0;
const unsigned long pauseDebounceTime = 250;

// Display timing
int startSequenceDisplayTime = 16000;
int sequenceDisplayTime = startSequenceDisplayTime;
int minimumSequenceDisplayTime = 4000;
int stepSequenceDisplayTime = 2000;
int resultDisplayTime = 3000;
int scoreDisplayTime = 2000;
int pauseTextDisplayTime = 1000;  // Time to display "PAuS" before menu

// Display buffer
char currentDisplay[4] = {' ', ' ', ' ', ' '};
int currentDigit = 0;
unsigned long lastDigitChange = 0;
const int digitDisplayTime = 5;  // ms per digit

// Joystick thresholds
const int joystickThresholdHigh = 800;
const int joystickThresholdLow = 200;
const int longPressTime = 1000;

// Blink rates
const int fastBlinkRate = 125;  // 4 Hz
const int slowBlinkRate = 500;  // 1 Hz

// Tone frequencies
const int toneTick = 1000;
const int toneClick = 1500;
const int toneSuccess = 2000;
const int toneError = 500;
const int toneDuration = 50;

const int eepromAddress = 100;

// Function prototypes
void handleButtonPress();
void updateMultiplexing();
void writeToShiftRegister(byte data);
byte getSegmentEncoding(char c);
void setDisplayText(const char* text);
void generateSequence();
void handleMenu();
void handleShowSequence();
void handleInputPhase();
void handleCheckAnswer();
void handleResult();
void handlePause();
void handleShowScore();
void handleShowPauseText();
void playTone(int frequency, int duration);
char getCharFromIndex(int index, alphaOrNumber type);
int getIndexFromChar(char c, alphaOrNumber type);

void setup() {
  Serial.begin(9600);
  
  highScore = EEPROM.read(eepromAddress);

  // Initialize SPI communication
  SPI.begin();
  
  // Setup input pins
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Setup digit control pins (common cathode - HIGH = off)
  for (int digit = 0; digit < displayDigitsNumber; digit++) {
    pinMode(displayDigits[digit], OUTPUT);
    digitalWrite(displayDigits[digit], HIGH);
  }
  
  // Setup joystick analog pins
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  
  // Setup SPI latch pin
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  // Attach interrupt for pause button
  attachInterrupt(digitalPinToInterrupt(PUSHBUTTON_PIN), handleButtonPress, FALLING);
  
  // Initialize display
  setDisplayText(textPlay);
  
  Serial.println("Simon Says game started!");
  Serial.println("Use the joystick to navigate the menu and play!");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Always update multiplexing
  updateMultiplexing();
  
  // Check for pause button press during game states
  if (pauseButtonPressed && (currentState == STATE_SHOW_SEQUENCE || currentState == STATE_INPUT_PHASE)) {
    pauseButtonPressed = false;  // Reset flag
    currentState = STATE_SHOW_PAUSE_TEXT;
    setDisplayText(textPause);
    displayStartTime = currentMillis;
    playTone(toneClick, toneDuration);
    Serial.println("Game paused...");
  }
  
  // State machine
  switch (currentState) {
    case STATE_MENU:
      handleMenu();
      break;
      
    case STATE_SHOW_SEQUENCE:
      handleShowSequence();
      break;
      
    case STATE_INPUT_PHASE:
      handleInputPhase();
      break;
      
    case STATE_CHECK_ANSWER:
      handleCheckAnswer();
      break;
      
    case STATE_RESULT:
      handleResult();
      break;
      
    case STATE_PAUSE:
      handlePause();
      break;
      
    case STATE_SHOW_SCORE:
      handleShowScore();
      break;
      
    case STATE_SHOW_PAUSE_TEXT:
      handleShowPauseText();
      break;
  }
}

void handleButtonPress() {
  unsigned long currentTime = millis();
  
  // Debounce the interrupt
  if (currentTime - lastPauseInterruptTime > pauseDebounceTime) {
    pauseButtonPressed = true;
    lastPauseInterruptTime = currentTime;
  }
}

void updateMultiplexing() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastDigitChange >= digitDisplayTime) {
    // Turn off current digit
    digitalWrite(displayDigits[currentDigit], HIGH);
    
    // Move to next digit
    currentDigit = (currentDigit + 1) % displayDigitsNumber;
    
    // Determine if we should display this digit (for blinking effects)
    bool shouldDisplay = true;
    
    if (currentState == STATE_INPUT_PHASE) {
      // Fast blink for selected digit (not locked)
      if (selectedDigitIndex == currentDigit && !digitLocked[currentDigit]) {
        if ((currentMillis / fastBlinkRate) % 2 == 0) {
          shouldDisplay = false;
        }
      }
      // Slow blink for locked digits
      else if (digitLocked[currentDigit]) {
        if ((currentMillis / slowBlinkRate) % 2 == 0) {
          shouldDisplay = false;
        }
      }
    }
    
    if (shouldDisplay) {
      // Get segment encoding for current character
      byte segments = getSegmentEncoding(currentDisplay[currentDigit]);
      
      // Write to shift register via SPI
      writeToShiftRegister(segments);
      
      // Turn on current digit (LOW for common cathode)
      digitalWrite(displayDigits[currentDigit], LOW);
    }
    
    lastDigitChange = currentMillis;
  }
}

void writeToShiftRegister(byte data) {
  digitalWrite(CS_PIN, LOW);    // Latch LOW - prepare to receive data
  SPI.transfer(data);            // Transfer byte via SPI
  digitalWrite(CS_PIN, HIGH);   // Latch HIGH - output the data
}

byte getSegmentEncoding(char c) {
  // Check character set
  for (int i = 0; i < charSetSize; i++) {
    if (charSet[i] == c) {
      return charSegmentEncoding[i];
    }
  }
  
  // Check number set
  for (int i = 0; i < numberSetSize; i++) {
    if (numberSet[i] == c) {
      return numberSegmentEncoding[i];
    }
  }
  
  // Return blank space by default
  return 0b00000000;
}

void setDisplayText(const char* text) {
  for (int i = 0; i < displayDigitsNumber; i++) {
    if (text[i] != '\0') {
      currentDisplay[i] = text[i];
    } else {
      currentDisplay[i] = ' ';
    }
  }
}

void generateSequence() {
  for (int i = 0; i < displayDigitsNumber; i++) {
    gameSequence[i] = charSet[random(charSetSize)];
  }
}

void handleMenu() {
  static unsigned long lastJoystickReading = 0;
  unsigned long currentMillis = millis();
  
  // Handle joystick navigation
  if (currentMillis - lastJoystickReading >= joystickDebounceTime) {
    int joystickYValue = analogRead(JOYSTICK_Y_PIN);
    
    if (joystickYValue > joystickThresholdHigh) {
      // Navigate down
      currentMenuItem = (menuItem)((currentMenuItem + 1) % 3);
      playTone(toneTick, toneDuration);
      
      switch (currentMenuItem) {
        case MENU_PLAY:
          setDisplayText(textPlay);
          break;
        case MENU_SCORE:
          setDisplayText(textScore);
          break;
        case MENU_STOP:
          setDisplayText(textStop);
          break;
      }
      
      lastJoystickReading = currentMillis;
    }
    else if (joystickYValue < joystickThresholdLow) {
      // Navigate up
      currentMenuItem = (menuItem)((currentMenuItem - 1 + 3) % 3);
      playTone(toneTick, toneDuration);
      
      switch (currentMenuItem) {
        case MENU_PLAY:
          setDisplayText(textPlay);
          break;
        case MENU_SCORE:
          setDisplayText(textScore);
          break;
        case MENU_STOP:
          setDisplayText(textStop);
          break;
      }
      
      lastJoystickReading = currentMillis;
    }
  }
  
  // Handle joystick button press
  if (digitalRead(JOYSTICK_BUTTON_PIN) == LOW && !joystickButtonPressed) {
    joystickButtonPressed = true;
    playTone(toneClick, toneDuration);
    
    switch (currentMenuItem) {
      case MENU_PLAY:
        currentRound = 1;
        sequenceDisplayTime = startSequenceDisplayTime;
        generateSequence();
        currentState = STATE_SHOW_SEQUENCE;
        displayStartTime = currentMillis;
        setDisplayText(gameSequence);
        Serial.println("Game started! Memorize the sequence...");
        break;
        
      case MENU_SCORE:
        {
          char scoreText[5];
          sprintf(scoreText, "%4d", highScore);
          setDisplayText(scoreText);
          currentState = STATE_SHOW_SCORE;
          displayStartTime = currentMillis;
          Serial.print("High score: ");
          Serial.println(highScore);
        }
        break;
        
      case MENU_STOP:
        setDisplayText(textStop);
        displayStartTime = currentMillis;
        currentState = STATE_SHOW_SCORE;
        break;
    }
  }
  
  if (digitalRead(JOYSTICK_BUTTON_PIN) == HIGH) {
    joystickButtonPressed = false;
  }
}

void handleShowSequence() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - displayStartTime >= sequenceDisplayTime) {
    // Transition to input phase
    currentState = STATE_INPUT_PHASE;
    cursorPosition = 0;
    selectedDigitIndex = -1;
    
    // Initialize player input with first character
    for (int digit = 0; digit < displayDigitsNumber; digit++) {
      playerInput[digit] = charSet[0];
      digitLocked[digit] = false;
    }
    
    setDisplayText(playerInput);
    Serial.println("Your turn! Enter the sequence...");
  }
}

void handleInputPhase() {
  static unsigned long lastJoystickReading = 0;
  static unsigned long buttonDownTime = 0;
  static bool buttonWasPressed = false;
  unsigned long currentMillis = millis();
  
  // Handle cursor movement (left/right) - only when no digit selected
  if (selectedDigitIndex == -1 && currentMillis - lastJoystickReading >= joystickDebounceTime) {
    int joystickXValue = analogRead(JOYSTICK_X_PIN);
    
    if (joystickXValue > joystickThresholdHigh) {
      // Move right
      cursorPosition = (cursorPosition + 1) % 4;
      playTone(toneTick, toneDuration);
      lastJoystickReading = currentMillis;
      Serial.print("Cursor at position: ");
      Serial.println(cursorPosition);
    }
    else if (joystickXValue < joystickThresholdLow) {
      // Move left
      cursorPosition = (cursorPosition - 1 + 4) % 4;
      playTone(toneTick, toneDuration);
      lastJoystickReading = currentMillis;
      Serial.print("Cursor at position: ");
      Serial.println(cursorPosition);
    }
  }
  
  // Handle character cycling (up/down) - only when digit selected
  if (selectedDigitIndex != -1 && !digitLocked[selectedDigitIndex]) {
    if (currentMillis - lastJoystickReading >= joystickDebounceTime) {
      int joystickYValue = analogRead(JOYSTICK_Y_PIN);
      
      if (joystickYValue > joystickThresholdHigh) {
        // Cycle up
        int currentIndex = getIndexFromChar(playerInput[selectedDigitIndex], ALPHA);
        currentIndex = (currentIndex + 1) % charSetSize;
        playerInput[selectedDigitIndex] = charSet[currentIndex];
        setDisplayText(playerInput);
        playTone(toneTick, toneDuration);
        lastJoystickReading = currentMillis;
      }
      else if (joystickYValue < joystickThresholdLow) {
        // Cycle down
        int currentIndex = getIndexFromChar(playerInput[selectedDigitIndex], ALPHA);
        currentIndex = (currentIndex - 1 + charSetSize) % charSetSize;
        playerInput[selectedDigitIndex] = charSet[currentIndex];
        setDisplayText(playerInput);
        playTone(toneTick, toneDuration);
        lastJoystickReading = currentMillis;
      }
    }
  }
  
  // Handle joystick button
  int buttonState = digitalRead(JOYSTICK_BUTTON_PIN);
  
  if (buttonState == LOW && !buttonWasPressed) {
    buttonWasPressed = true;
    buttonDownTime = currentMillis;
  }
  
  if (buttonState == LOW && buttonWasPressed) {
    // Check for long press
    if (currentMillis - buttonDownTime >= longPressTime && !joystickButtonLongPressed) {
      joystickButtonLongPressed = true;
      playTone(toneClick, toneDuration * 2);
      currentState = STATE_CHECK_ANSWER;
      Serial.println("Answer submitted!");
    }
  }
  
  if (buttonState == HIGH && buttonWasPressed) {
    // Short press handling
    if (!joystickButtonLongPressed) {
      if (selectedDigitIndex == -1) {
        // Select digit
        selectedDigitIndex = cursorPosition;
        digitLocked[selectedDigitIndex] = false;
        playTone(toneClick, toneDuration);
        Serial.print("Digit ");
        Serial.print(selectedDigitIndex);
        Serial.println(" selected. Use Up/Down to change character.");
      }
      else if (selectedDigitIndex == cursorPosition && !digitLocked[selectedDigitIndex]) {
        // Lock digit and deselect
        digitLocked[selectedDigitIndex] = true;
        selectedDigitIndex = -1;
        playTone(toneClick, toneDuration);
        Serial.print("Digit ");
        Serial.print(cursorPosition);
        Serial.println(" locked.");
      }
    }
    
    buttonWasPressed = false;
    joystickButtonLongPressed = false;
  }
}

void handleCheckAnswer() {
  bool correct = true;
  
  for (int i = 0; i < displayDigitsNumber; i++) {
    if (playerInput[i] != gameSequence[i]) {
      correct = false;
      break;
    }
  }
  
  if (correct) {
    playTone(toneSuccess, toneDuration * 3);
    currentRound++;
    
    if (currentRound - 1 > highScore) {
      highScore = currentRound - 1;
      EEPROM.update(eepromAddress, highScore);

    }
    
    sequenceDisplayTime = max(minimumSequenceDisplayTime, sequenceDisplayTime - stepSequenceDisplayTime);
    
    char scoreText[5];
    sprintf(scoreText, "%4d", currentRound - 1);
    setDisplayText(scoreText);
    
    Serial.print("Correct! Score: ");
    Serial.println(currentRound - 1);
    Serial.print("Next round display time: ");
    Serial.print(sequenceDisplayTime / 1000);
    Serial.println(" seconds");
    
    currentState = STATE_RESULT;
    displayStartTime = millis();
  }
  else {
    playTone(toneError, toneDuration * 3);
    setDisplayText(textError);
    
    Serial.println("Wrong! Game Over.");
    Serial.print("Final Score: ");
    Serial.println(currentRound - 1);
    
    currentState = STATE_RESULT;
    displayStartTime = millis();
  }
}

void handleResult() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - displayStartTime >= resultDisplayTime) {
    bool wasError = (currentDisplay[0] == 'E' && currentDisplay[1] == 'r' && currentDisplay[2] == 'r');
    
    if (!wasError && currentRound > 1) {
      // Continue to next round
      generateSequence();
      currentState = STATE_SHOW_SEQUENCE;
      displayStartTime = currentMillis;
      setDisplayText(gameSequence);
      
      Serial.print("Round ");
      Serial.print(currentRound);
      Serial.println(" - Memorize the new sequence!");
    }
    else {
      // Game over - return to menu
      currentState = STATE_MENU;
      currentMenuItem = MENU_PLAY;
      setDisplayText(textPlay);
      Serial.println("Game Over. Returning to menu...");
    }
  }
}

void handleShowPauseText() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - displayStartTime >= pauseTextDisplayTime) {
    // Transition to pause menu
    currentState = STATE_PAUSE;
    setDisplayText(textPlay);  // Start with PLAY option in pause menu
    currentMenuItem = MENU_PLAY;
    Serial.println("Navigate menu to resume or quit.");
  }
}

void handlePause() {
  static unsigned long lastJoystickReading = 0;
  unsigned long currentMillis = millis();
  
  // Navigate pause menu with joystick (same as main menu)
  if (currentMillis - lastJoystickReading >= joystickDebounceTime) {
    int joystickYValue = analogRead(JOYSTICK_Y_PIN);
    
    if (joystickYValue > joystickThresholdHigh) {
      // Navigate down
      currentMenuItem = (menuItem)((currentMenuItem + 1) % 3);
      playTone(toneTick, toneDuration);
      
      switch (currentMenuItem) {
        case MENU_PLAY:
          setDisplayText(textPlay);
          break;
        case MENU_SCORE:
          setDisplayText(textScore);
          break;
        case MENU_STOP:
          setDisplayText(textStop);
          break;
      }
      
      lastJoystickReading = currentMillis;
    }
    else if (joystickYValue < joystickThresholdLow) {
      // Navigate up
      currentMenuItem = (menuItem)((currentMenuItem - 1 + 3) % 3);
      playTone(toneTick, toneDuration);
      
      switch (currentMenuItem) {
        case MENU_PLAY:
          setDisplayText(textPlay);
          break;
        case MENU_SCORE:
          setDisplayText(textScore);
          break;
        case MENU_STOP:
          setDisplayText(textStop);
          break;
      }
      
      lastJoystickReading = currentMillis;
    }
  }
  
  // Handle joystick button press in pause menu
  if (digitalRead(JOYSTICK_BUTTON_PIN) == LOW && !joystickButtonPressed) {
    joystickButtonPressed = true;
    playTone(toneClick, toneDuration);
    
    switch (currentMenuItem) {
      case MENU_PLAY:
        // Resume game - return to showing sequence
        Serial.println("Resuming game...");
        currentState = STATE_SHOW_SEQUENCE;
        displayStartTime = currentMillis;
        setDisplayText(gameSequence);
        break;
        
      case MENU_SCORE:
        // Show high score
        {
          char scoreText[5];
          sprintf(scoreText, "%4d", highScore);
          setDisplayText(scoreText);
          currentState = STATE_SHOW_SCORE;
          displayStartTime = currentMillis;
          Serial.print("High score: ");
          Serial.println(highScore);
        }
        break;
        
      case MENU_STOP:
        // Stop game and return to main menu
        Serial.println("Game stopped. Returning to main menu...");
        currentState = STATE_MENU;
        currentMenuItem = MENU_PLAY;
        setDisplayText(textPlay);
        currentRound = 0;  // Reset game
        break;
    }
  }
  
  if (digitalRead(JOYSTICK_BUTTON_PIN) == HIGH) {
    joystickButtonPressed = false;
  }
}

void handleShowScore() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - displayStartTime >= scoreDisplayTime) {
    currentState = STATE_MENU;
    
    if (currentMenuItem == MENU_STOP) {
      currentMenuItem = MENU_PLAY;
    }
    
    switch (currentMenuItem) {
      case MENU_PLAY:
        setDisplayText(textPlay);
        break;
      case MENU_SCORE:
        setDisplayText(textScore);
        break;
      case MENU_STOP:
        setDisplayText(textStop);
        break;
    }
    
    Serial.println("Returned to menu.");
  }
}

void playTone(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
}

int getIndexFromChar(char c, alphaOrNumber type) {
  if (type == ALPHA) {
    for (int i = 0; i < charSetSize; i++) {
      if (charSet[i] == c) {
        return i;
      }
    }
  }
  else if (type == NUMBER) {
    for (int i = 0; i < numberSetSize; i++) {
      if (numberSet[i] == c) {
        return i;
      }
    }
  }
  
  return 0;
}

char getCharFromIndex(int index, alphaOrNumber type) {
  if (type == ALPHA) {
    if (index >= 0 && index < charSetSize) {
      return charSet[index];
    }
    return charSet[0];
  }
  else if (type == NUMBER) {
    if (index >= 0 && index < numberSetSize) {
      return numberSet[index];
    }
    return numberSet[0];
  }
  
  return ' ';
}
