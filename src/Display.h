/*
 * Class to drive a trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <Adafruit_Trellis.h>

// Delays [ms]
#define BLANK_DELAY    2500
#define BLINK_DELAY      20
#define HOLD_DELAY     1500
#define PAUSE_DELAY    1000

// Trellis LED brightness 1..15
#define BRIGHTNESS_SLEEPTICK  0
#define BRIGHTNESS_IDLE      10
#define BRIGHTNESS_PLAYING   10

// Trellis setup
#define NUMKEYS           16
#define TRELLIS_INT_PIN    1

// States
#define UNDEFINED       0
#define IDLE_LIGHT_UP   1
#define IDLE_WAIT_ON    2
#define IDLE_TURN_OFF   3
#define IDLE_WAIT_OFF   4


class Display {
  public:
    Display();
    void initialize();
    void tickMs();

    void onIdle();
    void onBlink(byte index, bool fast);

    void waitForNoKeyPressed();
    int getPressedKey();

    void onSleep();
    void onWakeup();
    void enableInterrupt(void (*isr)(void));
    void disableInterrupt();

  private:
    Adafruit_Trellis trellis;
    byte state;
    byte nextLED;
    unsigned long ticks;

    void onIdleLightUp();
    void onIdleWaitOn();
    void onIdleTurnOff();
    void onIdleWaitOff();
};

#endif
