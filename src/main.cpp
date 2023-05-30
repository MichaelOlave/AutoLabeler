#include <Arduino.h>
#include <AccelStepper.h>
#include <EEPROMex.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdlib.h>


// Define the button connections
#define RUN_BUTTON_PIN  4

// Define the stepper motor connections
#define STEP_PIN 2
#define DIR_PIN 5
#define ENABLE_PIN 8

// Define the motor steps per revolution
const int STEPS_PER_REV = 3200;

char inputString[16];
int customStep;

// Create an instance of AccelStepper
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

// Define Keypad
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {3, 6, 7, 11};    // Connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 9, 12, 13};       // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address and dimensions

int currentSelection = 0;

void saveCurrentSelection() {
  EEPROM.write(0, currentSelection);
}

void loadCurrentSelection() {
  currentSelection = EEPROM.read(0);
}

void clearInputString() {
  memset(inputString, 0, sizeof(inputString));
  customStep = 0;
}

void runSteps(int steps) {
  digitalWrite(ENABLE_PIN, LOW);  // Activates the Stepper motor
  stepper.moveTo(steps);  // Moves the Stepper motor a set number of steps
  stepper.runToPosition();
  stepper.setCurrentPosition(0);  // Resets the Stepper motor's current position to 0
  stepper.run();  // Runs the Stepper motor 
  digitalWrite(ENABLE_PIN, HIGH);  // Disables the Stepper motor
  delay(100);
}

void setup() {

  pinMode(RUN_BUTTON_PIN, INPUT_PULLUP);

  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
  stepper.setCurrentPosition(0);
  stepper.setPinsInverted(false, false, true);

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);

  keypad.setDebounceTime(50);
  keypad.setHoldTime(500);

  lcd.begin(16, 2); // Initialize the LCD with the specified dimensions
  lcd.setCursor(0, 0);
  lcd.print("Select an option:");
  lcd.setBacklight(20);

  // Load the current selection from EEPROM
  loadCurrentSelection();

  // Display the current selection on the LCD
  lcd.setCursor(0, 1);
  switch (currentSelection) {
    case 0:
      lcd.print("(Pre Roll Jars)");
      break;
    case 1:
      lcd.print("(Flour Jars)");
      break;
    case 2:
      lcd.print("(Pouch Front)");
      break;
    case 3:
      lcd.print("(Pouch Back)");
      break;
  }

  clearInputString();
}

void loop() {
  int runButtonState = digitalRead(RUN_BUTTON_PIN);

  char key = keypad.getKey();

  if (key) {
    switch (key) {
      case 'A':
        currentSelection = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selection:");
        lcd.setCursor(0, 1);
        lcd.print("(Pre Roll Jars)");
        break;
      case 'B':
        currentSelection = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selection:");
        lcd.setCursor(0, 1);
        lcd.print("(Flour Jars)");
        break;
      case 'C':
        currentSelection = 2;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selection:");
        lcd.setCursor(0, 1);
        lcd.print("(Pouch Front)");
        break;
      case 'D':
        currentSelection = 3;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selection:");
        lcd.setCursor(0, 1);
        lcd.print("(Pouch Back)");
        break;
      case '*':
        currentSelection = 4;
        clearInputString();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Custom Steps");
        break;
      case '#':
        if (customStep > 0) {
          inputString[customStep] = '\0';  // Null-terminate the input string
          int value = atoi(inputString);  // Convert input string to an integer
          clearInputString();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Value entered:");
          lcd.setCursor(0, 1);
          lcd.print(value);
          customStep = value;
        }
        break;
      default:
        if (customStep < sizeof(inputString) - 1) {
          inputString[customStep] = key;
          customStep++;
          lcd.setCursor(customStep - 1, 1);
          lcd.print(key);
        }
        break;
    }

    saveCurrentSelection();
  }

  switch (currentSelection) {
      case 0:  // Pre Roll Jars
        if (runButtonState == LOW) {
          runSteps(2610);
        }
        break;
      case 1:  // Flour Jars
        if (runButtonState == LOW) {
          runSteps(6400);
        }
        break;
      case 2:  // Pouch Front
        if (runButtonState == LOW) {
          runSteps(3200);
        }
        break;
      case 3:  // Pouch Back
        if (runButtonState == LOW) {
          runSteps(12800);
        }
        break;
      case 4:  // Custom Steps
        if (runButtonState == LOW) {
          runSteps(customStep);
        }
    }
}