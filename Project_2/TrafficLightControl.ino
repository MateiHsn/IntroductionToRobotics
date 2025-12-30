volatile bool buttonPressed = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200; // milliseconds

// traffic light finite state machine - related variables
int trafficState = 1;
bool pedestrianWarningFlashState = false;
unsigned long currentFlashTime = 0; // used for flashing LEDs
unsigned long currentBuzzTime = 0;
unsigned long stateStartTime = 0;

const unsigned long buzzerGoPeriod = 500;
const unsigned long buzzerWarningPeriod = 250;
const int buzzerFrequency = 2000;

const unsigned long lightFlashPeriod = 250;
bool lightFlashState = false;

bool buzzerState = false;


// pedestrian controls and signals
const int BUTTON_PIN = 3;
const int BUZZER_PIN = 10;
const int PEDESTRIAN_GREEN_PIN = 12;
const int PEDESTRIAN_RED_PIN = 11;

// car signals
const int CAR_RED_PIN = A0;
const int CAR_YELLOW_PIN = A1;
const int CAR_GREEN_PIN = A2;

// common signals
const int COUNTER_A_PIN = 8;
const int COUNTER_B_PIN = 7;
const int COUNTER_C_PIN = 6;
const int COUNTER_D_PIN = 5;
const int COUNTER_E_PIN = 4;
const int COUNTER_F_PIN = 9;
const int COUNTER_G_PIN = 2;

// traffic state delays (in milliseconds)
const unsigned long millisInASecond = 1000;
const unsigned long changeState1To2Time = 8000;
const unsigned long changeState2To3Time = 3000;
const unsigned long changeState3To4Time = 8000;
const unsigned long changeState4To1Time = 4000;

unsigned long numberToDisplay = 0;

bool countdownDelay = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PEDESTRIAN_GREEN_PIN, OUTPUT);
  pinMode(PEDESTRIAN_RED_PIN, OUTPUT);

  pinMode(COUNTER_A_PIN, OUTPUT);
  pinMode(COUNTER_B_PIN, OUTPUT);
  pinMode(COUNTER_C_PIN, OUTPUT);
  pinMode(COUNTER_D_PIN, OUTPUT);
  pinMode(COUNTER_E_PIN, OUTPUT);
  pinMode(COUNTER_F_PIN, OUTPUT);
  pinMode(COUNTER_G_PIN, OUTPUT);

  pinMode(CAR_RED_PIN, OUTPUT);
  pinMode(CAR_YELLOW_PIN, OUTPUT);
  pinMode(CAR_GREEN_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleInterrupt, FALLING);
}

void loop() {

  switch (trafficState) {
    case 1:
      digitalWrite(CAR_GREEN_PIN, HIGH);
      digitalWrite(CAR_RED_PIN, LOW);
      digitalWrite(CAR_YELLOW_PIN , LOW);

      digitalWrite(PEDESTRIAN_RED_PIN, HIGH);
      digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);

      if (buttonPressed == true && countdownDelay == false) {
        stateStartTime = millis();
        buttonPressed = false;
        countdownDelay = true;
      }

      if (millis() - stateStartTime > changeState1To2Time && countdownDelay == true) {
        trafficState = 2;
        stateStartTime = millis();
      }

      if (countdownDelay == true) {
//        numberToDisplay = (changeState1To2Time -  (millis() - stateStartTime)) / millisInASeconds;
//        displayNumbers(numberToDisplay);

        if ( millis() - currentFlashTime > lightFlashPeriod) {
          lightFlashState = !lightFlashState;
          currentFlashTime = millis();
        }
        digitalWrite(COUNTER_G_PIN, lightFlashState);

      }
      break;

    case 2:

      digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);
      digitalWrite(PEDESTRIAN_RED_PIN, HIGH);

      digitalWrite(CAR_YELLOW_PIN, HIGH);
      digitalWrite(CAR_GREEN_PIN, LOW);
      digitalWrite(CAR_RED_PIN, LOW);

      if (millis() - stateStartTime > changeState2To3Time) {
        trafficState = 3;
        stateStartTime = millis();
      }

      numberToDisplay = (changeState2To3Time -  (millis() - stateStartTime)) / millisInASecond;
      displayNumbers(numberToDisplay);

      break;

    case 3:

      digitalWrite(PEDESTRIAN_GREEN_PIN, HIGH);
      digitalWrite(PEDESTRIAN_RED_PIN, LOW);

      digitalWrite(CAR_RED_PIN, HIGH);
      digitalWrite(CAR_YELLOW_PIN, LOW);
      digitalWrite(CAR_GREEN_PIN, LOW);

      numberToDisplay = (changeState3To4Time -  (millis() - stateStartTime)) / millisInASecond;
      displayNumbers(numberToDisplay);

      if ( millis() - currentBuzzTime > buzzerGoPeriod) {
        buzzerState = !buzzerState;
        currentBuzzTime = millis();
      }

      if (buzzerState == true) {
        tone(BUZZER_PIN, buzzerFrequency, buzzerGoPeriod / 2);
      } else {
        noTone(BUZZER_PIN);
      }

      if ( millis() - stateStartTime > changeState3To4Time) {
        trafficState = 4;
        stateStartTime = millis();
      }
      break;

    case 4:

      numberToDisplay = (changeState4To1Time -  (millis() - stateStartTime)) / millisInASecond;
      displayNumbers(numberToDisplay);

      if ( millis() - currentFlashTime > lightFlashPeriod) {
        lightFlashState = !lightFlashState;
        currentFlashTime = millis();
      }

      digitalWrite(CAR_RED_PIN, HIGH);
      digitalWrite(PEDESTRIAN_GREEN_PIN, lightFlashState);

      digitalWrite(CAR_GREEN_PIN, LOW);
      digitalWrite(CAR_YELLOW_PIN, LOW);
      digitalWrite(PEDESTRIAN_RED_PIN, LOW);

      if ( millis() - currentBuzzTime > buzzerWarningPeriod) {
        buzzerState = !buzzerState;
        currentBuzzTime = millis();
      }

      if (buzzerState == true) {
        tone(BUZZER_PIN, buzzerFrequency, buzzerWarningPeriod / 2);
      } else {
        noTone(BUZZER_PIN);
      }

      if (millis() - stateStartTime > changeState4To1Time) {
        trafficState = 1;
        stateStartTime = millis();
      }
      countdownDelay = false;
      buttonPressed = false;
      break;
  }
}

void displayNumbers(unsigned long numberToDisplay) {
  switch (numberToDisplay) {
    case 0:
      digitalWrite(COUNTER_A_PIN, LOW);
      digitalWrite(COUNTER_B_PIN, LOW);
      digitalWrite(COUNTER_C_PIN, LOW);
      digitalWrite(COUNTER_D_PIN, LOW);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, LOW);
      digitalWrite(COUNTER_G_PIN, LOW);
      break;

    case 1:
      digitalWrite(COUNTER_A_PIN, LOW);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, LOW);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, LOW);
      digitalWrite(COUNTER_G_PIN, LOW);
      break;

    case 2:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, LOW);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, HIGH);
      digitalWrite(COUNTER_F_PIN, LOW);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;


    case 3:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, LOW);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;

    case 4:
      digitalWrite(COUNTER_A_PIN, LOW);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, LOW);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, HIGH);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;

    case 5:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, LOW);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, HIGH);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;

    case 6:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, LOW);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, HIGH);
      digitalWrite(COUNTER_F_PIN, HIGH);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;

    case 7:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, LOW);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, LOW);
      digitalWrite(COUNTER_G_PIN, LOW);
      break;

    case 8:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, HIGH);
      digitalWrite(COUNTER_F_PIN, HIGH);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;

    case 9:
      digitalWrite(COUNTER_A_PIN, HIGH);
      digitalWrite(COUNTER_B_PIN, HIGH);
      digitalWrite(COUNTER_C_PIN, HIGH);
      digitalWrite(COUNTER_D_PIN, HIGH);
      digitalWrite(COUNTER_E_PIN, LOW);
      digitalWrite(COUNTER_F_PIN, HIGH);
      digitalWrite(COUNTER_G_PIN, HIGH);
      break;
  }

}

void handleInterrupt() {
  static unsigned long interruptTime = 0;
  interruptTime = micros();
  if (interruptTime - lastInterruptTime > debounceDelay * millisInASecond) {
    buttonPressed = true;
  }
  lastInterruptTime = interruptTime;
}
