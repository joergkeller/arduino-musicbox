/*
 * Switch on/off one by one all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowRunning.h"

ShowRunning::ShowRunning(const Adafruit_Trellis& t) {
  trellis = &t;
}

void ShowRunning::initialize() {  
  state = IDLE_LIGHT_UP;
  nextLED = 0;
  ticks = 0L;
  
  trellis->setBrightness(BRIGHTNESS_IDLE);
  trellis->blinkRate(HT16K33_BLINK_OFF);
  trellis->clear();
  trellis->writeDisplay();
}

void ShowRunning::tickMs() {
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

void ShowRunning::onIdleLightUp() {
  trellis->setLED(nextLED);
  trellis->writeDisplay();
  nextLED = (nextLED + 1) % NUMKEYS;
  if (nextLED == 0) {
    state = IDLE_WAIT_ON;
  }
}

void ShowRunning::onIdleWaitOn() {
  state = IDLE_TURN_OFF;
}

void ShowRunning::onIdleTurnOff() {
  trellis->clrLED(nextLED);
  trellis->writeDisplay();
  nextLED = (nextLED + 1) % NUMKEYS;
  if (nextLED == 0) {
    state = IDLE_WAIT_OFF;
  }
}

void ShowRunning::onIdleWaitOff() {
  trellis->clear(); // just to clean up
  trellis->writeDisplay();
  state = IDLE_LIGHT_UP;
}

