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
#include <Adafruit_NeoTrellis.h>
#include "ShowAbstract.h"

// Delays [ms]
#define STEP_DELAY   30

// NeoTrellis LED brightness 1..255
#define BRIGHTNESS_MIN    0
#define BRIGHTNESS_MAX   64

// States
#define IDLE_UP      1
#define IDLE_DOWN    2


class ShowPulsing : public ShowAbstract {
  public:
    ShowPulsing(Adafruit_NeoTrellis& t);
    virtual void initialize();
    virtual void tickMs();

  private:
    Adafruit_NeoTrellis& trellis;
    byte state;
    uint8_t brightness;
    unsigned long ticks;

    void showWheel();
    void onIdleUp();
    void onIdleDown();
};

#endif
