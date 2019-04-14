/*
 * Constantly switch on all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef ShowAlwaysOn_h
#define ShowAlwaysOn_h

#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>
#include "ShowAbstract.h"

// Trellis LED brightness 1..15
#define BRIGHTNESS_IDLE   15


class ShowAlwaysOn : public ShowAbstract {
  public:
    ShowAlwaysOn(Adafruit_NeoTrellis& t);
    virtual void initialize();
    virtual void tickMs() {}

  private:
    Adafruit_NeoTrellis& trellis;
};

#endif
