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
#define READ_DELAY   50

// Trellis wiring
#define INTPIN 1

// States
#define IDLE_LIGHT_UP 1
#define IDLE_TURN_OFF 2
#define IDLE_WAIT_OFF 3
#define PLAY_SELECTED 4


Adafruit_Trellis trellis = Adafruit_Trellis();
unsigned long nextIdleTick = millis();
unsigned long nextReadTick = millis();
byte state = IDLE_LIGHT_UP;
byte nextLED = 0;
byte playingAlbum;


void setup() {
  Serial.begin(14400);
  while (!Serial) delay(1);
  Serial.println("MusicBox setup");
  
  // INT pin requires a pullup
  pinMode(INTPIN, INPUT_PULLUP);

  initializeTrellis(false);
}

void initializeTrellis(boolean delay) {
  trellis.begin(0x70);
  //trellis.readSwitches(); // ignore already pressed switches

  trellis.blinkRate(HT16K33_BLINK_OFF);
  trellis.clear();
  trellis.writeDisplay();

  Serial.println("Trellis initialized");

  state = IDLE_LIGHT_UP;
  nextLED = 0;
  nextIdleTick = millis() + (delay ? 1200 : 0);
}

void loop() {
  unsigned long now = millis();
  if (nextIdleTick <= now) {
    nextIdleTick += tickIdleShow();
  }
  if (nextReadTick <= now) {
    nextReadTick += tickReadKeys();
  }
}

unsigned int tickIdleShow() {
  // Light up all keys in order
  if (state == IDLE_LIGHT_UP) {
    trellis.setLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      state = IDLE_TURN_OFF;
    }
    return BLINK_DELAY;

  // Turn off all keys in order
  } else if (state == IDLE_TURN_OFF) {
    trellis.clrLED(nextLED);
    trellis.writeDisplay();
    nextLED = (nextLED + 1) % NUMKEYS;
    if (nextLED == 0) {
      state = IDLE_WAIT_OFF;
      return 2000;
    }
    return BLINK_DELAY;

  // Wait in darkness
  } else if (state == IDLE_WAIT_OFF) {
    //Serial.print(".");
    trellis.clear(); // just to clean up
    state = IDLE_LIGHT_UP;
    return BLINK_DELAY;
  }
}

unsigned int tickReadKeys() {
  if (trellis.readSwitches()) {
    for (byte i = 0; i < NUMKEYS; i++) {
      if (trellis.justPressed(i)) {
        onKey(i);
        break;
      }
    }
  }
  return READ_DELAY;
}

void onKey(byte index) {
  if (state == PLAY_SELECTED && playingAlbum == index) {
    // Pressed playing key again
    Serial.print("Stopped album #"); Serial.println(index);
    initializeTrellis(true);
  } else {
    // Pressed key to play a new album
    trellis.clear();
    trellis.setLED(index);
    trellis.blinkRate(HT16K33_BLINK_1HZ);
    trellis.writeDisplay();
  
    state = PLAY_SELECTED;
    playingAlbum = index;
   
    Serial.print("Playing album #"); Serial.println(index);
  }
}

