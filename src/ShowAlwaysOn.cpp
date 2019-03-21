/*
 * Constantly switch on all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowAlwaysOn.h"

ShowAlwaysOn::ShowAlwaysOn(const Adafruit_Trellis& t) 
: trellis(t) {}

void ShowAlwaysOn::initialize() {  
  trellis.setBrightness(BRIGHTNESS_IDLE);
  trellis.blinkRate(HT16K33_BLINK_OFF);
  for (byte i = 0; i < NUMKEYS; i++) {
    trellis.setLED(i);
  }
  trellis.writeDisplay();
}
