/*
 * Pulse (fade-in/fade-out) all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowAlternating.h"

ShowAlternating::ShowAlternating(Adafruit_Trellis& t)
: trellis(t) {}

void ShowAlternating::initialize() {  
  state = IDLE_UP;
  even = HIGH;
  brightness = BRIGHTNESS_MIN;
  ticks = 0L;
  
  trellis.setBrightness(brightness);
  trellis.blinkRate(HT16K33_BLINK_OFF);
  toggleLED();
}

void ShowAlternating::toggleLED() {
  for (byte i = 0; i < NUMKEYS; i++) {
    bool on = i / 4 % 2 ? even : !even;
    if (i % 2 != on) {
      trellis.setLED(i);
    } else {
      trellis.clrLED(i);
    }
  }
  trellis.writeDisplay();
}

void ShowAlternating::tickMs() {
  ticks++;
  if (state == IDLE_UP && ticks >= STEP_DELAY) {
    ticks = 0L;
    onIdleUp();
  } else if (state == IDLE_DOWN && ticks >= STEP_DELAY) {
    ticks = 0L;
    onIdleDown();
  } else if (state == IDLE_OFF && ticks >= STEP_DELAY) {
    ticks = 0L;
    onIdleOff();
  }
}

void ShowAlternating::onIdleUp() {
  trellis.setBrightness(++brightness);
  if (brightness >= BRIGHTNESS_MAX) {
    state = IDLE_DOWN;
  }
}

void ShowAlternating::onIdleDown() {
  trellis.setBrightness(--brightness);
  if (brightness <= BRIGHTNESS_MIN) {
    state = IDLE_OFF;
    trellis.clear();
    trellis.writeDisplay();
  }
}

void ShowAlternating::onIdleOff() {
  state = IDLE_UP;
  even = !even;
  toggleLED();
}

