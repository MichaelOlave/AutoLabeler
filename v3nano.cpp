#include <Arduino.h>
#include <AccelStepper.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdlib.h>


// Define the button connections
#define RUN_BUTTON_PIN  13

// Define the stepper motor connections
#define STEP_PIN 11
#define DIR_PIN 10
#define ENABLE_PIN 9

// Define the motor steps per revolution
const int STEPS_PER_REV = 3200;

char inputString[16];
size_t customStep;

// Counter settings
int counter = 0;
bool buttonState = HIGH;    // Current state of the button
bool lastButtonState = HIGH;// Previous state of the button
unsigned long debounceDelay = 50;  // Delay in milliseconds for debouncing
unsigned long lastDebounceTime = 0; // Last time the button state changed

// Main menu settings
unsigned long keyPressStartTime = 0;
bool menuRequested = false;
const unsigned long holdDuration = 5000;
const char clearKey = '0';

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

byte rowPins[ROWS] = {7, 6, 8, 12=};    // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2};       // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD address and dimensions

int currentSelection = 0;

void saveCurrentSelection() {
  EEPROM.write(0, currentSelection);
}

void loadCurrentSelection() {
  currentSelection = EEPROM.read(0);
}

void clearInputString() {
  memset(inputString, 0, sizeof(inputString));
  customStep = 0u;
}

void runSteps(size_t steps) {
  digitalWrite(ENABLE_PIN, LOW);  // Activates the Stepper motor
  stepper.moveTo(steps);  // Moves the Stepper motor a set number of steps
  stepper.runToPosition();
  stepper.setCurrentPosition(0);  // Resets the Stepper motor's current position to 0
  stepper.run();  // Runs the Stepper motor 
  digitalWrite(ENABLE_PIN, HIGH);  // Disables the Stepper motor
  delay(100);
}

void mainMenu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pre Rolls: A");
  lcd.setCursor(0,1);
  lcd.print("Flour Jars: B");
  lcd.setCursor(0,2);
  lcd.print("Pouch Front: C");
  lcd.setCursor(0,3);
  lcd.print("Pouch Back: D");
}

void selectionMenu(String sel) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(sel);
  lcd.setCursor(0,2);
  lcd.print("Number Labeled:");
  lcd.setCursor(0,3);
  lcd.print(0);
}

void counterFun() {
  // Read the state of the button
  buttonState = digitalRead(RUN_BUTTON_PIN);

  // Check if the button state has changed and debouncing time has passed
  if (buttonState != lastButtonState && millis() - lastDebounceTime > debounceDelay) {
    // Update the last debounce time
    lastDebounceTime = millis();

    // Check if the button is pressed (LOW state)
    if (buttonState == LOW) {
      counter++;                          // Increment the counter
      lcd.setCursor(0, 3);                 // Set the LCD cursor position
      lcd.print("       ");                // Clear the previous counter value
      lcd.setCursor(0, 3);                 // Set the LCD cursor position again
      lcd.print(counter);                  // Print the updated counter value on the LCD
    }
  }

  // Update the last button state
  lastButtonState = buttonState;
}

void menuRequest(int i) {
  if(i == clearKey) {
    keyPressStartTime = millis();
    menuRequested = false;
  }

  if(i == clearKey && !menuRequested && (millis() - keyPressStartTime >= holdDuration)) {
    mainMenu();
    menuRequested = true;
  }
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

  lcd.begin(20, 4); // Initialize the LCD with the specified dimensions 
  lcd.setBacklight(20);
  mainMenu();

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

  menuRequest(key);

  if (key) {
    switch (key) {
      case 'A':
        currentSelection = 0;
        selectionMenu("Pre Rolls");
        break;
      case 'B':
        currentSelection = 1;
        selectionMenu("Flour Jars");
        break;
      case 'C':
        currentSelection = 2;
        selectionMenu("Pouch Front");
        break;
      case 'D':
        currentSelection = 3;
        selectionMenu("Pouch Back");
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
        if (customStep < static_cast<unsigned int>(sizeof(inputString) - 1)) {
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
          runSteps(6350);
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
  lcd.setCursor(0,3);
  counterFun();
}
