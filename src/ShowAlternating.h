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
#include <Adafruit_NeoTrellis.h>
#include "ShowAbstract.h"

// Delays [ms]
#define STEP_DELAY   30

// Trellis LED brightness 0..255
#define BRIGHTNESS_MIN    0
#define BRIGHTNESS_MAX   64

// States
#define IDLE_UP      1
#define IDLE_DOWN    2
#define IDLE_OFF     3

class ShowAlternating : public ShowAbstract {
  public:
    ShowAlternating(Adafruit_NeoTrellis& t);
    virtual void initialize();
    virtual void tickMs();

  private:
    Adafruit_NeoTrellis& trellis;
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
