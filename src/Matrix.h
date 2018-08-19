/*
 * Class to drive a trellis display matrix.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef Matrix_h
#define Matrix_h

#include <Arduino.h>
#include <Adafruit_Trellis.h>
#include "ShowAbstract.h"
#include "ShowAlwaysOn.h"
#include "ShowRunning.h"
#include "ShowPulsing.h"
#include "ShowAlternating.h"

// Define the idle mode (ShowAlwaysOn, ShowRunning, ShowPulsing, ShowAlternating)
#define SHOW_CLASS   ShowAlternating

// Trellis LED brightness 1..15
#define BRIGHTNESS_PLAYING   15

// Trellis setup
#define TRELLIS_INT_PIN    1


class Matrix {
  public:
    Matrix();
    void initialize();
    void tickMs();

    void idle();
    void blink(byte index, bool fast);

    void waitForNoKeyPressed();
    int getPressedKey();

    void sleep();
    void wakeup();
    void enableInterrupt(void (*isr)(void));
    void disableInterrupt();

  private:
    const Adafruit_Trellis trellis = Adafruit_Trellis();
    const ShowAbstract& show = SHOW_CLASS(trellis);
    bool isIdle = false;
};

#endif
