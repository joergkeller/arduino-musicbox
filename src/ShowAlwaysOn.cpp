/*
 * Constantly switch on all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "ShowAlwaysOn.h"

ShowAlwaysOn::ShowAlwaysOn(Adafruit_NeoTrellis& t) 
: trellis(t) {}

void ShowAlwaysOn::initialize() {  
  for (byte i = 0; i < NUMKEYS; i++) {
    trellis.pixels.setPixelColor(i, wheel(i, BRIGHTNESS_IDLE));
  }
  trellis.pixels.show();
}
