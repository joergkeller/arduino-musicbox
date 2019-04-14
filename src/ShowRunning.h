/*
 * Switch on/off one by one all lights of the trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef ShowRunning_h
#define ShowRunning_h

#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>
#include "ShowAbstract.h"

// Delays [ms]
#define BLANK_DELAY    1500
#define BLINK_DELAY      25
#define HOLD_DELAY     1500
#define PAUSE_DELAY    1000

// Trellis LED brightness 1..15
#define BRIGHTNESS_IDLE   15

// States
#define IDLE_LIGHT_UP   1
#define IDLE_WAIT_ON    2
#define IDLE_TURN_OFF   3
#define IDLE_WAIT_OFF   4


class ShowRunning : public ShowAbstract {
  public:
    ShowRunning(Adafruit_NeoTrellis& t);
    virtual void initialize();
    virtual void tickMs();

  private:
    Adafruit_NeoTrellis& trellis;
    byte state;
    byte nextLED;
    unsigned long ticks;

    void onIdleLightUp();
    void onIdleWaitOn();
    void onIdleTurnOff();
    void onIdleWaitOff();
};

#endif
