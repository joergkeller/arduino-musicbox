/*
 * Pulse (fade-in/fade-out) all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowPulsing.h"

ShowPulsing::ShowPulsing(Adafruit_Trellis& t) 
: trellis(t) {}

void ShowPulsing::initialize() {  
  state = IDLE_UP;
  brightness = BRIGHTNESS_MIN;
  ticks = 0L;
  
  trellis.setBrightness(brightness);
  trellis.blinkRate(HT16K33_BLINK_OFF);
  for (byte i = 0; i < NUMKEYS; i++) {
    trellis.setLED(i);
  }
  trellis.writeDisplay();
}

void ShowPulsing::tickMs() {
  ticks++;
  if (state == IDLE_UP && ticks >= STEP_DELAY) {
    ticks = 0L;
    onIdleUp();
  } else if (state == IDLE_DOWN && ticks >= STEP_DELAY) {
    ticks = 0L;
    onIdleDown();
  }
}

void ShowPulsing::onIdleUp() {
  trellis.setBrightness(++brightness);
  if (brightness >= BRIGHTNESS_MAX) {
    state = IDLE_DOWN;
  }
}

void ShowPulsing::onIdleDown() {
  trellis.setBrightness(--brightness);
  if (brightness <= BRIGHTNESS_MIN) {
    state = IDLE_UP;
  }
}
