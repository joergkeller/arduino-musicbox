/*
 * Pulse (fade-in/fade-out) all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef ShowAlternating_h
#define ShowAlternating_h

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
#define IDLE_OFF     3

class ShowAlternating : public ShowAbstract {
  public:
    ShowAlternating(Adafruit_Trellis& t);
    virtual void initialize();
    virtual void tickMs();

  private:
    Adafruit_Trellis& trellis;
    byte state;
    byte even;
    byte brightness;
    unsigned long ticks;

    void toggleLED();
    void onIdleUp();
    void onIdleDown();
    void onIdleOff();
};

#endif
