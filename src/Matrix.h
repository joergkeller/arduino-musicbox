/*
 * Class to drive a neo-trellis display matrix.
 *
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef Matrix_h
#define Matrix_h

#include <Arduino.h>
#include "Adafruit_NeoTrellis.h"
#include "ShowAbstract.h"
#include "ShowAlwaysOn.h"
#include "ShowRunning.h"
#include "ShowPulsing.h"
#include "ShowAlternating.h"

// Define the idle mode (ShowAlwaysOn, ShowRunning, ShowPulsing, ShowAlternating)
#define SHOW_CLASS   ShowPulsing

// Trellis LED brightness 0..255
#define BRIGHTNESS_PLAYING   96

// Delay to read neo-trellis keys
#define KEY_READ_DELAY  20 // ms, the neo-trellis has a resolution of around 60 Hz approx. 17 ms

// Trellis setup
#define TRELLIS_INT_PIN    1

/*
 * Provides access to a neo-trellis 4x4 board with LED output and key input.
 * The RGB-LED can have any color/brightness and keys can trigger events.
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
    void attachKeyPress(TrellisCallback (*handler)(keyEvent));

    void sleep();
    void wakeup();
    void enableInterrupt(void (*isr)(void));
    void disableInterrupt();

  private:
    Adafruit_NeoTrellis trellis = Adafruit_NeoTrellis();
    SHOW_CLASS show = SHOW_CLASS(trellis);
    bool isIdle = false;
    byte readTicks = 0;
};

#endif
