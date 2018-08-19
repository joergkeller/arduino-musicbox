/*
 * Class to drive a trellis display matrix.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "Matrix.h"

Matrix::Matrix() {
  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(TRELLIS_INT_PIN, INPUT_PULLUP);
}

void Matrix::initialize() {
  // clear display
  trellis.begin(0x70);
  trellis.clear();
  trellis.writeDisplay();

  // ignore already pressed switches
  trellis.readSwitches(); 
  
  Serial.println("Trellis initialized");
}

void Matrix::tickMs() {
  if (isIdle) {
    show.tickMs();
  }
}

void Matrix::idle() {
  isIdle = true;
  show.initialize();
}

void Matrix::blink(byte index, bool fast) {
  isIdle = false;
  trellis.clear();
  trellis.setBrightness(BRIGHTNESS_PLAYING);
  trellis.setLED(index);
  trellis.blinkRate(fast ? HT16K33_BLINK_1HZ : HT16K33_BLINK_HALFHZ);
  trellis.writeDisplay();
}

void Matrix::waitForNoKeyPressed() {
  unsigned long timeout = millis() + 1000;
  while (millis() <= timeout) { 
    byte keyPressed = 0;
    trellis.readSwitches();
    for (byte i = 0; i < NUMKEYS; i++) {
      keyPressed += trellis.isKeyPressed(i) ? 1 : 0;
    }
    if (keyPressed == 0) return;
    delay(50);
  }
}

int Matrix::getPressedKey() {
  if (trellis.readSwitches()) {
    for (byte i = 0; i < NUMKEYS; i++) {
      if (trellis.justPressed(i)) {
        return i;
      }
    }
  }
  return -1;
}

void Matrix::sleep() {
  isIdle = false;
  trellis.clear();
  trellis.writeDisplay();
  trellis.sleep();
}

void Matrix::wakeup() {
  isIdle = false;
  trellis.wakeup();
}

void Matrix::enableInterrupt(void (*isr)(void)) {
  attachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN), isr, LOW);
}

void Matrix::disableInterrupt() {
  detachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN));
}

