/*
 * Pulse (fade-in/fade-out) all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef ShowPulsing_h
#define ShowPulsing_h

#include <Arduino.h>
#include <Adafruit_Trellis.h>
#include "ShowAbstract.h"

// Delays [ms]
#define STEP_DELAY   300

// Trellis LED brightness 1..15
#define BRIGHTNESS_MIN    0
#define BRIGHTNESS_MAX   15

// Trellis setup
#define NUMKEYS   16

// States
#define IDLE_UP      1
#define IDLE_DOWN    2


class ShowPulsing : public ShowAbstract {
  public:
    ShowPulsing(const Adafruit_Trellis& t);
    virtual void initialize();
    virtual void tickMs();

  private:
    const Adafruit_Trellis& trellis;
    byte state;
    byte brightness;
    unsigned long ticks;

    void onIdleUp();
    void onIdleDown();
};

#endif
