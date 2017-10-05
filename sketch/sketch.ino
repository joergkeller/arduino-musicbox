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
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_Trellis.h>

#define NUMKEYS 4
#define BLINK_DELAY 100
#define READ_DELAY   50

// Trellis pin setup
#define INTPIN 1

// Feather/Wing pin setup
#define MUSIC_RESET   -1     // VS1053 reset pin (not used!)
#define CARD_CS        5     // Card chip select pin
#define MUSIC_CS       6     // VS1053 chip select pin (output)
#define MUSIC_DCS     10     // VS1053 Data/command select pin (output)
#define MUSIC_DREQ     9     // VS1053 Data request, ideally an Interrupt pin (not possible on 32u4)

// States
#define IDLE_LIGHT_UP 1
#define IDLE_TURN_OFF 2
#define IDLE_WAIT_OFF 3
#define PLAY_SELECTED 4

/*************************************************** 
 * Variables
 ****************************************************/

Adafruit_Trellis trellis = Adafruit_Trellis();
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET, MUSIC_CS, MUSIC_DCS, MUSIC_DREQ, CARD_CS);

unsigned long nextReadTick = millis();
unsigned long nextIdleTick = millis();
byte state = IDLE_LIGHT_UP;
byte nextLED = 0;
byte playingAlbum;


/*************************************************** 
 * Setup
 ****************************************************/
void setup() {
  Serial.begin(14400);
  Serial.println("MusicBox setup");
  
  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(INTPIN, INPUT_PULLUP);

  initializeCard();
  initializePlayer();
  initializeTrellis(0);
}

/*************************************************** 
 * Loop
 ****************************************************/
void loop() {
  unsigned long now = millis();
  if (nextReadTick > now + (1000L * 60L * 60L)) {
    Serial.println("Rollover timer ticks!");
    nextReadTick = now;
    nextIdleTick = now;
  }
  if (nextReadTick <= now) {
    nextReadTick += tickReadKeys();
  }
  if (nextIdleTick <= now) {
    nextIdleTick += tickIdleShow();
  }
}

void initializeTrellis(int delay) {
  trellis.begin(0x70);
  //trellis.readSwitches(); // ignore already pressed switches

  trellis.blinkRate(HT16K33_BLINK_OFF);
  trellis.clear();
  trellis.writeDisplay();

  Serial.println("Trellis initialized");

  state = IDLE_LIGHT_UP;
  nextLED = 0;
  nextIdleTick = millis() + delay;
}

void initializeCard() {
  if (!SD.begin(CARD_CS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD initialized");
}

void initializePlayer() {
  if (!player.begin()) { 
     Serial.println(F("Couldn't find VS1053"));
     while (1);
  }
  player.softReset();
  player.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  player.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
  Serial.println(F("VS1053 initialized"));
  
  // Set volume for left, right channels. lower numbers == louder volume!
  //   0: max volume
  // 130: audible 
  // 254: min volume
  // 255: analog off
  player.setVolume(64, 64);
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
    stopPlayingAlbum();
    Serial.print("Stopped album #"); Serial.println(index);
    initializeTrellis(1200);
  } else {
    // Pressed key to play a new album
    if (state == PLAY_SELECTED) stopPlayingAlbum();
    trellis.clear();
    trellis.setLED(index);
    trellis.blinkRate(HT16K33_BLINK_1HZ);
    trellis.writeDisplay();
  
    state = PLAY_SELECTED;
    playingAlbum = index;
    playAlbumNew(index);
   
    Serial.print("Playing album #"); Serial.println(index);
  }
}

void playAlbumNew(byte index) {
  switch (index) {
    case 0: player.startPlayingFile("track001.mp3"); break;
    case 1: player.startPlayingFile("track002.mp3"); break;
    case 2: player.startPlayingFile("track003.mp3"); break;
    case 3: player.sineTest(0x44, 500); break;
  }
}

void stopPlayingAlbum() {
  player.stopPlaying();
  delay(20);
}

