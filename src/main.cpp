#include <Arduino.h>
#include <AccelStepper.h>
#include <EEPROMex.h>

// EEPROM global variables
#define NUMSELECTIONS 4
unsigned int addressButton1StateBits;
int currentSelection;
boolean button1Pressed = 0;
boolean previousButton1Pressed = 0;
unsigned long debounceCount;
unsigned long blinkMillis;
unsigned int blinkInterval;

// Define RGB LED Light
#define LED_R 7
#define LED_G 6
#define LED_B 3

// Define the button connections
#define RUN_BUTTON_PIN  4
#define LED_BUTTON_PIN  12

int runbuttonstate;

// Define the stepper motor connections
#define STEP_PIN 2
#define DIR_PIN 5
#define ENABLE_PIN 8

// Define the motor steps per revolution
const int STEPS_PER_REV = 3200;

// Create an instance of AccelStepper
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

void runSteps(int Steps) {
  digitalWrite(ENABLE_PIN, LOW);  // Activates the Stepper motor
  stepper.moveTo(Steps);  // Moves the Stepper motor a set number of steps
  stepper.runToPosition();
  stepper.setCurrentPosition(0);  // Resets the Stepper motors current position to 0
  stepper.run();  // Runs the Stepper motor 
  digitalWrite(ENABLE_PIN, HIGH);  // Disables the Stepper motor
  delay(100);
}

void setColor(int R, int G, int B) {
  analogWrite(LED_R,  R);
  analogWrite(LED_G, G);
  analogWrite(LED_B, B);
}

byte readEESelection() {
  // count through the bits of storage until we find a bit set to 1
  byte i;
  for (i = 0; i < 32; i++) {
    byte byteNum, bitNum;
    byteNum = i / 8;
    bitNum = i % 8;
    boolean bitValue = EEPROM.readBit(addressButton1StateBits + byteNum, bitNum);
    if (bitValue) {
      break;
    }
  }
  // return the modulus of the bit number, that is the selection number
  return i % NUMSELECTIONS;
}

void updateEESelection() {
  while (readEESelection() != currentSelection) {
    // count through the bits of storage until we find a bit set to 1
    // and set that bit to 0 
    byte i, byteNum, bitNum;
    for (i = 0; i < 32; i++) {
      byteNum = i / 8;
      bitNum = i % 8;
      if (EEPROM.readBit(addressButton1StateBits + byteNum, bitNum) == 1) {
        EEPROM.updateBit(addressButton1StateBits + byteNum, bitNum, 0);
        break;
      }
      // if we don't find a bit set to 1, set all to 1 to start search over
      if (i == 31) { // a bit to clear was not found before the last bit
        for (i = 0; i < 4; i++) {
          EEPROM.updateByte(addressButton1StateBits + i, 0xFF);
        }
      }
    }
  } // while loop repeats until the bits are set so they match selection number
}

void setup() {
  //Serial.begin(9600);
  //Serial.print("\nRestored Selection ");
  //Serial.println(currentSelection);
  // EEPROM settings 
  EEPROM.setMaxAllowedWrites(400);
  EEPROM.setMemPool(256, EEPROMSizeATmega328);
  addressButton1StateBits = EEPROM.getAddress(4);
  currentSelection = readEESelection();

  // RGB pin settings
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Set button input
  pinMode(RUN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUTTON_PIN, INPUT_PULLUP);

  // Set the maximum speed and acceleration
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
  // Sets the Stepper motors current position to 0
  stepper.setCurrentPosition(0);
  // Set the motor direction to clockwise
  stepper.setPinsInverted(false, false, true);

  // Enable the motor outputs
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);  // HIGH disables the Stepper motor
}

void loop() {
  runbuttonstate = digitalRead(RUN_BUTTON_PIN);  // Determins if the button is pushed

  // read the button no more often than once every 20 ms, debouncing
  if (millis() > 20 && debounceCount < millis() - 20) {
    debounceCount = millis();
    button1Pressed = !digitalRead(12);
  }
  // if the button has been pressed, increment and store selection
  if (previousButton1Pressed != button1Pressed) {
    previousButton1Pressed = button1Pressed;
    // only take action when button is pressed, not when released
    if (button1Pressed) {
      // turn off the LED for the old selection
      //digitalWrite(currentSelection + 2, 1);
      currentSelection++;
      currentSelection %= NUMSELECTIONS;
      // turn on the LED for the new selection
      //digitalWrite(currentSelection + 2, 0);
       Serial.print("Current Selection ");
       Serial.println(currentSelection);
      updateEESelection();
    }
  }
  // Pre Roll Jars
  if(currentSelection == 0) {
    setColor(255, 255, 0);
    if(runbuttonstate == LOW) {
    runSteps(2610);
  }
  }
  // Flour Jars
  else if(currentSelection == 1) {
    setColor(255, 0, 0);
    if(runbuttonstate == LOW) {
    runSteps(6400);
  }
  }
  // Pouch Front
  else if(currentSelection == 2) {
    setColor(0, 255, 0);
    if(runbuttonstate == LOW) {
    runSteps(3200);
  }
  }
  // Pouch Back
  else if(currentSelection == 3) {
    setColor(0, 0, 255);
    if(runbuttonstate == LOW) {
    runSteps(3600);
  }
  }
}

