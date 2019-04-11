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
#define SHOW_CLASS   ShowPulsing

// Trellis LED brightness 1..15
#define BRIGHTNESS_PLAYING   15

// Trellis setup
#define TRELLIS_INT_PIN    1

/*
 * Provides access to a trellis 4x4 board with LED output and key input.
 * The LED can be switched on/off, have a level of brightness and can blink in two frequencies. Keys can be polled.
 */
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
    Adafruit_Trellis trellis = Adafruit_Trellis();
    SHOW_CLASS show = SHOW_CLASS(trellis);
    bool isIdle = false;
};

#endif
