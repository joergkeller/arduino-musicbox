/***************************************************
  Arduino control code for a music maker board and a trellis keypad.

  The control code shall
     read trellis keys (/)
     on key press play a music file from the micro SD card from a specific folder and blink the respective key (/)
     on pressing the same key or when playback stops jump to the next file in the folder, until all music is played (/)
     read the rotary decoder to change the volume, the maximum volume can be configured (/)
     detect when a headphone is plugged in and mute the amplifier for the internal speakers (/)
     switch the music box off automatically after an idle period

  Todos:
   - Audio off when idle (volume 255,255) -> proper file padding needed (?)

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
#include <LowPower.h>


// Delays [ms]
#define IDLE_PAUSE    2000
#define BLINK_DELAY    100
#define READ_DELAY      50
#define IDLE_TIMEOUT (1000L * 60L)
#define ROLLOVER_GAP (1000L * 60L * 60L)

// Trellis LED brightness 1..15
#define BRIGHTNESS_SLEEPTICK  0
#define BRIGHTNESS_IDLE      10
#define BRIGHTNESS_PLAYING   10

// Trellis setup
#define NUMKEYS 4
#define INT_PIN 1

// Feather/Wing pin setup
#define MUSIC_RESET_PIN   12     // VS1053 reset pin (not used!)
#define CARD_CS_PIN        5     // Card chip select pin
#define MUSIC_CS_PIN       6     // VS1053 chip select pin (output)
#define MUSIC_DCS_PIN     10     // VS1053 Data/command select pin (output)
#define MUSIC_DREQ_PIN     9     // VS1053 Data request, ideally an Interrupt pin (not possible on 32u4)

// Amplifier pin setup
#define AMPLIFIER_ENABLE_PIN   11   // Enable both amplifier channels
#define HEADSET_LEVEL_PIN      A3   // Voltage level indicates headset plugin
#define HEADSET_THRESHOLD     100   // Plugged-in: ~20, Unplugged: ~890

// Rotary Encoder with Switch and LED
#define ENCODER_A           A0
#define ENCODER_B           A1
#define ENCODER_SWITCH      A2
#define GREEN_LED_PIN       A4
#define BLUE_LED_PIN        A5

// States
#define IDLE_LIGHT_UP   1
#define IDLE_TURN_OFF   2
#define IDLE_WAIT_OFF   3
#define PLAY_SELECTED   4
#define TIMEOUT_WAIT    5 

// Volume
#define VOLUME_MIN       130    // max. 254 moderation
#define VOLUME_MAX         0    // min. 0 moderation
#define VOLUME_DIRECTION  +1
#define VOLUME_OFF       255    // 255 = switch audio off, TODO avoiding cracking noise, maybe correct stuffing needed when stop

/***************************************************
   Variables
 ****************************************************/

Adafruit_Trellis trellis = Adafruit_Trellis();
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET_PIN, MUSIC_CS_PIN, MUSIC_DCS_PIN, MUSIC_DREQ_PIN, CARD_CS_PIN);
ClickEncoder encoder = ClickEncoder(ENCODER_A, ENCODER_B, ENCODER_SWITCH, 2, LOW);


unsigned long nextReadTick = millis() + 1;
unsigned long nextIdleTick = millis() + 1;
unsigned long nextTimeoutTick = 0;
byte state = IDLE_LIGHT_UP;
byte nextLED = 0;
byte playingAlbum;
File album;
int volume = 64;
boolean headset = false;
boolean headsetFirstMeasure = false;


/***************************************************
   Setup
 ****************************************************/
void setup() {
  initializeAmplifier();

  Serial.begin(14400);
  while (!Serial && millis() < nextIdleTick + 2000);
  Serial.println("MusicBox setup");

  initializeCard();
  initializePlayer();
  initializeTrellis();
  initializeTimer();
  initializeSwitchLed();                    

  onEnterIdle(0);
}

void initializeSwitchLed() {
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BLUE_LED_PIN, HIGH);
}

void initializeAmplifier() {
  pinMode(AMPLIFIER_ENABLE_PIN, OUTPUT);
  pinMode(HEADSET_LEVEL_PIN, INPUT_PULLUP);
  enableAmplifier(false);
}

void initializeCard() {
  if (!SD.begin(CARD_CS_PIN)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD initialized");

  File root = SD.open("/");
  while (true) {
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
  pinMode(MUSIC_RESET_PIN, OUTPUT);
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

void initializeTrellis() {
  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(INT_PIN, INPUT_PULLUP);

  trellis.begin(0x70);
  //trellis.readSwitches(); // ignore already pressed switches
  Serial.println("Trellis initialized");
}

void initializeTimer() {
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  Serial.println("Timer initialized");
}

void onEnterIdle(unsigned int delay) {
  trellis.setBrightness(BRIGHTNESS_IDLE);
  trellis.blinkRate(HT16K33_BLINK_OFF);
  trellis.clear();
  trellis.writeDisplay();

  nextReadTick = millis() + 1;
  nextIdleTick = millis() + 1 + delay;
  nextTimeoutTick = millis() + IDLE_TIMEOUT;

  state = IDLE_LIGHT_UP;
  nextLED = 0;
}


/***************************************************
   Interrupt-Handler
 ****************************************************/
void timerIsr() {
  encoder.service();
}

/***************************************************
   Loop
 ****************************************************/
void loop() {
  unsigned long now = millis();

  // Rollover millis since start
  if (nextReadTick > now + ROLLOVER_GAP) {
    Serial.println("Rollover timer ticks!");
    nextReadTick = now + 1;
    nextIdleTick = now + 1;
    nextTimeoutTick = nextTimeoutTick == 0 ? 0 : now + IDLE_TIMEOUT;
  }

  // Next ticks
  if (0 < nextTimeoutTick && nextTimeoutTick <= now) {
    tickIdleTimeout();
  }
  if (0 < nextReadTick && nextReadTick <= now) {
    nextReadTick = now + tickReadKeys();
  }
  if (0 < nextIdleTick && nextIdleTick <= now) {
    nextIdleTick = now + tickIdleShow();
  }

  // Check for finished track
  if (state == PLAY_SELECTED) {
    if (player.stopped()) onTryNextTrack();
  }
}

unsigned long tickIdleShow() {
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
    trellis.clear(); // just to clean up
    trellis.writeDisplay();
    state = IDLE_LIGHT_UP;
    return BLINK_DELAY;
  }
}

void tickIdleTimeout() {
  Serial.println("Timeout!");
  onTimeout();

  attachInterrupt(digitalPinToInterrupt(INT_PIN), trellisIsr, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 

  if (state == TIMEOUT_WAIT) {
    // temporary wakeup after 8s, no interrupt called
    onWatchdogPing();
  } else {
    onSleepWake();
  }
}

void onTimeout() {
  trellis.clear();
  trellis.writeDisplay();
  trellis.sleep();
  
  state = TIMEOUT_WAIT;
}

void onWatchdogPing() {
  digitalWrite(BLUE_LED_PIN, LOW); // LED on
  delay(1);
  digitalWrite(BLUE_LED_PIN, HIGH); // LED off
}

void trellisIsr() {
  detachInterrupt(digitalPinToInterrupt(INT_PIN));
  state = IDLE_LIGHT_UP;
}

void onSleepWake() {
  trellis.wakeup();
  waitForNoKeyPressed();
  onEnterIdle(0);
}

void waitForNoKeyPressed() {
  unsigned long timeout = millis() + 1000;
  while (millis() <= timeout) { 
    byte keyPressed = 0;
    trellis.readSwitches();
    for (byte i = 0; i < NUMKEYS; i++) {
      keyPressed += trellis.isKeyPressed(i) ? 1 : 0;
    }
    if (keyPressed == 0) return;
    delay(20);
  }
}

unsigned long tickReadKeys() {
  // Read trellis keys
  if (trellis.readSwitches()) {
    for (byte i = 0; i < NUMKEYS; i++) {
      if (trellis.justPressed(i)) {
        if (state == TIMEOUT_WAIT) onSleepWake();
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

  // Read headset level
  int audioLevel = analogRead(HEADSET_LEVEL_PIN);
  if (headset != headsetFirstMeasure && (audioLevel > HEADSET_THRESHOLD) != headsetFirstMeasure) {
    // confirmed change
    onHeadsetInserted(audioLevel < HEADSET_THRESHOLD);
    Serial.print("Confirmed change, headset "); Serial.println(headsetFirstMeasure);
  } else if ((audioLevel < HEADSET_THRESHOLD) != headsetFirstMeasure) {
    // there seems to be a change
    headsetFirstMeasure = (audioLevel < HEADSET_THRESHOLD);
    Serial.print("Possible change, headset "); Serial.println(headsetFirstMeasure);
  }

  return READ_DELAY;
}

void onKey(byte index) {
  nextTimeoutTick = 0;

  // Same key pressed again
  if (state == PLAY_SELECTED && playingAlbum == index) {
    stopPlaying();
    onTryNextTrack();

  // Some (other) key pressed
  } else {
    if (state == PLAY_SELECTED) stopPlaying();
    enablePlayer(true);
    enableAmplifier(!headset);
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
  onEnterIdle(1200);
}

void onHeadsetInserted(boolean plugged) {
  headset = plugged;
  enableAmplifier(!headset);
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
  trellis.setBrightness(BRIGHTNESS_PLAYING);
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
  if (enable) {
    digitalWrite(MUSIC_RESET_PIN, HIGH);
    player.reset();
    player.setVolume(volume, volume);
  } else {
    player.setVolume(VOLUME_OFF, VOLUME_OFF);
    digitalWrite(MUSIC_RESET_PIN, LOW);
  }
}

void enableAmplifier(boolean enable) {
  digitalWrite(AMPLIFIER_ENABLE_PIN, enable);
}

