/*************************************************** 
  Arduino control code for a music maker board and a trellis keypad.

  The control code shall
   * read trellis keys
   * on key press play a music file from the micro SD card from a specific folder and blink the respective key
   * on pressing the same key or when playback stops jump to the next file in the folder, until all music is played
   * read the rotary decoder to change the volume, the maximum volume can be configured
   * detect when a headphone is plugged in and mute the amplifier for the internal speakers
   * switch the music box off automatically after an idle period

  Written by Jörg Keller, Switzerland  
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_Trellis.h>

#define NUMKEYS 4
#define BLINK_DELAY 100

// Trellis wiring
#define INTPIN 1

// Idle show
#define LIGHT_UP 1
#define TURN_OFF 2
#define WAIT_OFF 3


Adafruit_Trellis trellis = Adafruit_Trellis();
unsigned long nextIdleTick = millis();
int idleShowState = LIGHT_UP;
int nextLED = 0;

void setup() {
  Serial.begin(14400);
  while (!Serial) delay(1);
  Serial.println("MusicBox setup");
  
  // INT pin requires a pullup
  pinMode(INTPIN, INPUT_PULLUP);

  initializeTrellis();
}

void initializeTrellis() {
  trellis.begin(0x70);
  trellis.readSwitches(); // ignore already pressed switches
  trellis.clear();
  trellis.writeDisplay();
  Serial.println("Trellis initialized");
}

void loop() {
  unsigned long now = millis();
  if (nextIdleTick <= now) {
    nextIdleTick += tickIdleShow();
  }
}

unsigned int tickIdleShow() {
  // Light up all keys in order
  if (idleShowState == LIGHT_UP) {
    trellis.setLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      idleShowState = TURN_OFF;
    }
    return BLINK_DELAY;

  // Turn off all keys in order
  } else if (idleShowState == TURN_OFF) {
    trellis.clrLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      idleShowState = WAIT_OFF;
      return 2000;
    }
    return BLINK_DELAY;

  // Wait in darkness
  } else if (idleShowState == WAIT_OFF) {
    //Serial.print(".");
    trellis.clear(); // just to clean up
    idleShowState = LIGHT_UP;
    return BLINK_DELAY;
  }
}

