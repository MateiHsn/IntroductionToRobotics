// Pins connected to the potentiometers
const int redLEDPotPin = A0;
const int greenLEDPotPin = A1;
const int blueLEDPotPin = A2;

// Pins connected to each LED
const int redLEDPin = 9;
const int greenLEDPin = 10;
const int blueLEDPin = 11;

// Value range for the ADC input
const int adcMinInput = 0;
const int adcMaxInput = 1023;

// Value range for the PWM output duty cycle ( 0-255 == 0-100 )
const int minOutputValue = 0;
const int maxOutputValue = 255;

void setup() {
  
  pinMode(redLEDPotPin, INPUT);
  pinMode(greenLEDPotPin, INPUT);
  pinMode(blueLEDPotPin, INPUT);

  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  
}

void loop() {

  // Logic for controlling the red LED
  int redLEDPotInputValue = analogRead(redLEDPotPin);
  int redOutputValue = map(redLEDPotInputValue, adcMinInput, adcMaxInput, minOutputValue, maxOutputValue);
  analogWrite(redLEDPin, redOutputValue);

  // Logic for controlling the green LED
  int greenLEDPotInputValue = analogRead(greenLEDPotPin);
  int greenOutputValue = map(greenLEDPotInputValue, adcMinInput, adcMaxInput, minOutputValue, maxOutputValue);
  analogWrite(greenLEDPin, greenOutputValue);

  // Logic for controlling the blue LED
  int blueLEDPotInputValue = analogRead(blueLEDPotPin);
  int blueOutputValue = map(blueLEDPotInputValue, adcMinInput, adcMaxInput, minOutputValue, maxOutputValue);
  analogWrite(blueLEDPin, blueOutputValue);
  
}
