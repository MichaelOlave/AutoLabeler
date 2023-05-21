#include <Arduino.h>
#include <EEPROMex.h>

#define NUMSELECTIONS 4
// global variables
unsigned int addressButton1StateBits;
int currentSelection;
boolean button1Pressed = 0;
boolean previousButton1Pressed = 0;
unsigned long debounceCount;
unsigned long blinkMillis;
unsigned int blinkInterval;

void setup() {
  //Serial.begin(57600);
  // initialize EEPROM settings
  EEPROM.setMaxAllowedWrites(400);
  EEPROM.setMemPool(256, EEPROMSizeATmega328);
  addressButton1StateBits = EEPROM.getAddress(4);
  currentSelection = readEESelection();
  // Serial.print("\nRestored Selection ");
  // Serial.println(currentSelection);
  pinMode(12, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(2, 1);
  digitalWrite(3, 1);
  digitalWrite(4, 1);
  digitalWrite(5, 1);
  digitalWrite(13, 1);
  // turn on the LED for the restored selection
  digitalWrite(currentSelection + 2, 0);
}  // end of setup

void loop() {
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
      digitalWrite(currentSelection + 2, 1);
      currentSelection++;
      currentSelection %= NUMSELECTIONS;
      // turn on the LED for the new selection
      digitalWrite(currentSelection + 2, 0);
      // Serial.print("Current Selection ");
      // Serial.println(currentSelection);
      updateEESelection();
    }
  }

  // Here in the loop is where we can do different things depending on
  // what selection was made with the button.
  // In this case we blink the Arduino on-board LED at different intervals
  // depending on the selection.
  // This also demonstrates main loop processing without delay blocking.
  if (currentSelection == 0) {
    // Dim the LED by blinking it rapidly, bit-banging PWM
    if (digitalRead(13)) {
      blinkInterval = 0;
    }
    else {
      blinkInterval = 10;
    }
  }
  else if (currentSelection == 1) {
    // Blink once per second
    blinkInterval = 500;
  }
  else if (currentSelection == 2) {
    // Blink every 2 seconds
    blinkInterval = 1000;
  }
  else if (currentSelection == 3) {
    // Quick flash every 1.5 seconds, this is also bit-banging PWM, very slow
    if (digitalRead(13)) {
      blinkInterval = 50;
    }
    else {
      blinkInterval = 1450;
    }
  }
  if (millis() > blinkInterval && blinkMillis < millis() - blinkInterval) {
    blinkMillis = millis();
    digitalWrite(13, !digitalRead(13));
  }

}  // end of loop

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