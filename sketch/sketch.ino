/*************************************************** 
  Arduino control code for a music maker board and a trellis keypad.

  The control code shall
   * read trellis keys (/)
   * on key press play a music file from the micro SD card from a specific folder and blink the respective key (/)
   * on pressing the same key or when playback stops jump to the next file in the folder, until all music is played (/)
   * read the rotary decoder to change the volume, the maximum volume can be configured
   * detect when a headphone is plugged in and mute the amplifier for the internal speakers
   * switch the music box off automatically after an idle period

  Todos:
   - Serial waits for a max. time
   - Audio off when idle (volume 255,255)

  Written by Jörg Keller, Switzerland  
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_Trellis.h>
#include <ClickEncoder.h>
#include <TimerOne.h>


#define NUMKEYS 4
#define IDLE_PAUSE  2000
#define BLINK_DELAY  100
#define READ_DELAY    50
#define ROLLOVER_GAP (1000L * 60L * 60L)

// Trellis pin setup
#define INTPIN 1

// Feather/Wing pin setup
#define MUSIC_RESET   -1     // VS1053 reset pin (not used!)
#define CARD_CS        5     // Card chip select pin
#define MUSIC_CS       6     // VS1053 chip select pin (output)
#define MUSIC_DCS     10     // VS1053 Data/command select pin (output)
#define MUSIC_DREQ     9     // VS1053 Data request, ideally an Interrupt pin (not possible on 32u4)

// Amplifier pin setup
#define AMPLIFIER_ENABLE 11   // Enable both amplifier channels

// States
#define IDLE_LIGHT_UP 1
#define IDLE_TURN_OFF 2
#define IDLE_WAIT_OFF 3
#define PLAY_SELECTED 4

// Volume
#define VOLUME_MIN       130    // max. 254 moderation
#define VOLUME_MAX        20    // min. 0 moderation
#define VOLUME_DIRECTION  -1
#define VOLUME_OFF       254    // 255 = switch audio off, TODO avoiding cracking noise, maybe correct stuffing needed when stop

/*************************************************** 
 * Variables
 ****************************************************/

Adafruit_Trellis trellis = Adafruit_Trellis();
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET, MUSIC_CS, MUSIC_DCS, MUSIC_DREQ, CARD_CS);
ClickEncoder encoder = ClickEncoder(A1, A0, A2, 2, LOW);


unsigned long nextReadTick = millis();
unsigned long nextIdleTick = millis();
byte state = IDLE_LIGHT_UP;
byte nextLED = 0;
byte playingAlbum;
File album;
int volume = 64;


/*************************************************** 
 * Setup
 ****************************************************/
void setup() {
  initializeAmplifier();

  Serial.begin(14400);
  while (!Serial && millis() < nextIdleTick + 2000);
  Serial.println("MusicBox setup");
  
  initializeCard();
  initializePlayer();
  initializeTrellis(0);
  initializeTimer();
}

void initializeAmplifier() {
  pinMode(AMPLIFIER_ENABLE, OUTPUT);
  enableAmplifier(false);
}

void initializeCard() {
  if (!SD.begin(CARD_CS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD initialized");

  File root = SD.open("/");
  while(true) {
    File entry = root.openNextFile();
    if (!entry) break;
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
  }
}

void initializePlayer() {
  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(INTPIN, INPUT_PULLUP);

  if (!player.begin()) { 
     Serial.println(F("Couldn't find VS1053"));
     while (1);
  }
  player.softReset();
  //player.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  player.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
  Serial.println(F("VS1053 initialized"));
  
  enablePlayer(false);
}

void initializeTrellis(int delay) {
  trellis.begin(0x70);
  //trellis.readSwitches(); // ignore already pressed switches
  Serial.println("Trellis initialized");

  trellis.blinkRate(HT16K33_BLINK_OFF);
  trellis.clear();
  trellis.writeDisplay();

  nextIdleTick = millis() + delay;
}

void initializeTimer() {
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  Serial.println("Timer initialized");
}

/*************************************************** 
 * Interrupt-Handler
 ****************************************************/
void timerIsr() {
  encoder.service();
}

/*************************************************** 
 * Loop
 ****************************************************/
void loop() {
  unsigned long now = millis();

  // Rollover millis since start
  if (nextReadTick > now + ROLLOVER_GAP) {
    Serial.println("Rollover timer ticks!");
    nextReadTick = now;
    nextIdleTick = now;
  }

  // Next ticks
  if (nextReadTick <= now) {
    nextReadTick += tickReadKeys();
  }
  if (nextIdleTick <= now) {
    nextIdleTick += tickIdleShow();
  }

  // Check for finished track
  if (state == PLAY_SELECTED) {
    if (player.stopped()) onTryNextTrack();
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
      return IDLE_PAUSE;
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
  // Read trellis keys
  if (trellis.readSwitches()) {
    for (byte i = 0; i < NUMKEYS; i++) {
      if (trellis.justPressed(i)) {
        onKey(i);
        break;
      }
    }
  }

  // Read encoder position
  if (state == PLAY_SELECTED) {
    int encoderChange = encoder.getValue() * VOLUME_DIRECTION;
    if (encoderChange != 0) {
      changeVolume(encoderChange);
    }
  }
  
  return READ_DELAY;
}

void onKey(byte index) {
  // Same key pressed again
  if (state == PLAY_SELECTED && playingAlbum == index) {
    stopPlaying();
    onTryNextTrack();

  // Some (other) key pressed
  } else {
    if (state == PLAY_SELECTED) stopPlaying();
    enablePlayer(true);
    enableAmplifier(true);
    onStartFirstTrack(index);
  }
}

void onStartFirstTrack(byte index) {
  playingAlbum = index;
  blinkSelected(playingAlbum);
  openNewAlbum(playingAlbum);
  if (playNextTrack()) {
    Serial.print("Playing album #"); Serial.println(playingAlbum);
    state = PLAY_SELECTED;
  } else {
    if (album && album.isDirectory()) album.close();
    Serial.print("Failed album #"); Serial.println(playingAlbum);
    onStopPlaying();
  }
}

void onTryNextTrack() {
  if (playNextTrack()) {
    Serial.print("Playing next track album #"); Serial.println(playingAlbum);
    blinkSelected(playingAlbum);
    state = PLAY_SELECTED;
  } else {
    if (album && album.isDirectory()) album.close();
    Serial.print("Ended album #"); Serial.println(playingAlbum);
    onStopPlaying();
  }
}

void onStopPlaying() {
  enableAmplifier(false);
  enablePlayer(false);
  initializeTrellis(1200);
  state = IDLE_LIGHT_UP;
  nextLED = 0;
}

// Set volume for left, right channels. lower numbers == louder volume!
//   0: max volume
//  20: cracks on amplified loudspeaker
// 130: audible 
// 254: min volume
// 255: analog off
void changeVolume(int encoderChange) {
  volume += encoderChange;
  volume = max(VOLUME_MAX, min(volume, VOLUME_MIN));
  Serial.print("Set Volume "); Serial.println(volume);
  player.setVolume(volume, volume);
}

void blinkSelected(byte index) {
  trellis.clear();
  trellis.setLED(index);
  trellis.blinkRate(HT16K33_BLINK_1HZ);
  trellis.writeDisplay();
}

boolean openNewAlbum(byte index) {
  int albumNr = index + 1;
  String albumTemplate = "ALBUM";
  String albumName = albumTemplate + albumNr;
  if (album && album.isDirectory()) album.close();
  album = SD.open(albumName);
}

boolean playNextTrack() {
  if (!album || !album.isDirectory()) return false;
  File track = album.openNextFile();
  if (!track) return false;

  String albumName = album.name();
  String dirPath = albumName + '/';
  String filePath = dirPath + track.name();
  Serial.print("Next track: "); Serial.println(filePath);
  track.close();

  player.startPlayingFile(filePath.c_str());
  return true;
}

void stopPlaying() {
  player.stopPlaying();
  delay(20);
}

void enablePlayer(boolean enable) {
  int vol = enable ? volume : VOLUME_OFF;
  player.setVolume(vol, vol);
}

void enableAmplifier(boolean enable) {
  digitalWrite(AMPLIFIER_ENABLE, enable);
}

