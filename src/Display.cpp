/*
 * Class to drive a trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "Display.h"

Display::Display() {
  trellis = Adafruit_Trellis();
  state = UNDEFINED;
}

void Display::initialize() {
  // clear display
  trellis.begin(0x70);
  trellis.clear();
  trellis.writeDisplay();

  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(TRELLIS_INT_PIN, INPUT_PULLUP);

  // ignore already pressed switches
  trellis.readSwitches(); 
  Serial.println("Trellis initialized");
}

void Display::tickMs() {
  ticks++;
  if (state == IDLE_LIGHT_UP && ticks >= BLINK_DELAY) {
    ticks = 0L;
    onIdleLightUp();
  } else if (state == IDLE_WAIT_ON && ticks >= HOLD_DELAY) {
    ticks = 0L;
    onIdleWaitOn();
  } else if (state == IDLE_TURN_OFF && ticks >= BLINK_DELAY) {
    ticks = 0L;
    onIdleTurnOff();
  } else if (state == IDLE_WAIT_OFF && ticks >= BLANK_DELAY) {
    ticks = 0L;
    onIdleWaitOff();
  }
}

void Display::onIdleLightUp() {
  trellis.setLED(nextLED);
  trellis.writeDisplay();
  nextLED = (nextLED + 1) % NUMKEYS;
  if (nextLED == 0) {
    state = IDLE_WAIT_ON;
  }
}

void Display::onIdleWaitOn() {
  state = IDLE_TURN_OFF;
}

void Display::onIdleTurnOff() {
  trellis.clrLED(nextLED);
  trellis.writeDisplay();
  nextLED = (nextLED + 1) % NUMKEYS;
  if (nextLED == 0) {
    state = IDLE_WAIT_OFF;
  }
}

void Display::onIdleWaitOff() {
  trellis.clear(); // just to clean up
  trellis.writeDisplay();
  state = IDLE_LIGHT_UP;
}

void Display::onIdle() {
  trellis.setBrightness(BRIGHTNESS_IDLE);
  trellis.blinkRate(HT16K33_BLINK_OFF);
  trellis.clear();
  trellis.writeDisplay();

  state = IDLE_LIGHT_UP;
  nextLED = 0;
  ticks = 0L;
}

void Display::onBlink(byte index, bool fast) {
  state = UNDEFINED;
  trellis.clear();
  trellis.setBrightness(BRIGHTNESS_PLAYING);
  trellis.setLED(index);
  trellis.blinkRate(fast ? HT16K33_BLINK_1HZ : HT16K33_BLINK_HALFHZ);
  trellis.writeDisplay();
}

void Display::waitForNoKeyPressed() {
  unsigned long timeout = millis() + 1000;
  while (millis() <= timeout) { 
    byte keyPressed = 0;
    trellis.readSwitches();
    for (byte i = 0; i < NUMKEYS; i++) {
      keyPressed += trellis.isKeyPressed(i) ? 1 : 0;
    }
    if (keyPressed == 0) return;
    delay(20);
  }
}

int Display::getPressedKey() {
  if (trellis.readSwitches()) {
    for (byte i = 0; i < NUMKEYS; i++) {
      if (trellis.justPressed(i)) {
        return i;
      }
    }
  }
  return -1;
}

void Display::onSleep() {
  state = UNDEFINED;
  trellis.clear();
  trellis.writeDisplay();
  trellis.sleep();
}

void Display::onWakeup() {
  state = UNDEFINED;
  trellis.wakeup();
}

void Display::enableInterrupt(void (*isr)(void)) {
  attachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN), isr, LOW);
}

void Display::disableInterrupt() {
  detachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN));
}

