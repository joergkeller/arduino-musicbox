/*
 * Pulse (fade-in/fade-out) all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowAlternating.h"

ShowAlternating::ShowAlternating(Adafruit_NeoTrellis& t)
: trellis(t) {}

void ShowAlternating::initialize() {  
  state = IDLE_UP;
  even = HIGH;
  brightness = BRIGHTNESS_MIN;
  ticks = 0L;
  
  toggleLED();
}

void ShowAlternating::toggleLED() {
  for (byte i = 0; i < NUMKEYS; i++) {
    bool on = i / 4 % 2 ? even : !even;
    if (i % 2 != on) {
      trellis.pixels.setPixelColor(i, wheel(i, brightness));
    } else {
      trellis.pixels.setPixelColor(i, 0);
    }
  }
  trellis.pixels.show();
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
  ++brightness;
  toggleLED();
  if (brightness >= BRIGHTNESS_MAX) {
    state = IDLE_DOWN;
  }
}

void ShowAlternating::onIdleDown() {
  --brightness;
  toggleLED();
  if (brightness <= BRIGHTNESS_MIN) {
    state = IDLE_OFF;
  }
}

void ShowAlternating::onIdleOff() {
  state = IDLE_UP;
  even = !even;
  toggleLED();
}
