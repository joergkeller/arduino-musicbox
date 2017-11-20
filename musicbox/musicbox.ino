/***************************************************
  Arduino control code for a music maker board and a trellis keypad.

  The control code shall
     read trellis keys (/)
     on key press play a music file from the micro SD card from a specific folder and blink the respective key (/)
     on pressing the same key or when playback stops jump to the next file in the folder, until all music is played (/)
     read the rotary decoder to change the volume, the maximum volume can be configured (/)
     press the rotary decoder to pause/resume/stop/sleep (/)
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
#define IDLE_PAUSE     2000
#define BLINK_DELAY     100
#define PAUSE_DELAY    1000
#define READ_DELAY       50
#define IDLE_TIMEOUT  (1000L * 60L)
#define PAUSE_TIMEOUT (1000L * 60L *  5L)
#define ROLLOVER_GAP  (1000L * 60L * 60L)

// Trellis LED brightness 1..15
#define BRIGHTNESS_SLEEPTICK  0
#define BRIGHTNESS_IDLE      10
#define BRIGHTNESS_PLAYING   10

// Trellis setup
#define NUMKEYS            4
#define TRELLIS_INT_PIN    1

// Feather/Wing pin setup
#define MUSIC_RESET_PIN   12     // VS1053 reset pin (not used!)
#define CARD_CS_PIN        5     // Card chip select pin
#define MUSIC_CS_PIN       6     // VS1053 chip select pin (output)
#define MUSIC_DCS_PIN     10     // VS1053 Data/command select pin (output)
#define MUSIC_DREQ_PIN     9     // VS1053 Data request, ideally an Interrupt pin (not possible on 32u4)

// Amplifier pin setup
#define AMPLIFIER_ENABLE_PIN   11   // Enable both amplifier channels
#define HEADPHONE_LEVEL_PIN    A2   // Voltage level indicates headphone plugin
#define HEADPHONE_THRESHOLD   100   // Plugged-in: ~20, Unplugged: ~890

// Rotary Encoder with Switch and LED
#define ENCODER_A_PIN       A0
#define ENCODER_B_PIN       A1
#define ENCODER_SWITCH_PIN   0
#define RED_LED_PIN         A3
#define GREEN_LED_PIN       A4
#define BLUE_LED_PIN        A5

// States
#define IDLE_LIGHT_UP   1
#define IDLE_TURN_OFF   2
#define IDLE_WAIT_OFF   3
#define PLAY_SELECTED   4
#define PLAY_PAUSED     5
#define TIMEOUT_WAIT    6 

// Volume
#define SPEAKER_VOLUME_MIN         130    // max. 254 moderation
#define SPEAKER_VOLUME_MAX           5    // min. 0 moderation
#define HEADPHONE_VOLUME_MIN       150    // max. 254 moderation
#define HEADPHONE_VOLUME_MAX        60    // min. 0 moderation
#define VOLUME_DIRECTION            +1
#define VOLUME_OFF                 255    // 255 = switch audio off, TODO avoiding cracking noise, maybe correct stuffing needed when stop

/***************************************************
   Variables
 ****************************************************/

Adafruit_Trellis trellis = Adafruit_Trellis();
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET_PIN, MUSIC_CS_PIN, MUSIC_DCS_PIN, MUSIC_DREQ_PIN, CARD_CS_PIN);
ClickEncoder encoder = ClickEncoder(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_SWITCH_PIN, 2, LOW, HIGH);


unsigned long nextReadTick = millis() + 1;
unsigned long nextIdleTick = millis() + 1;
unsigned long nextTimeoutTick = 0;
byte state = IDLE_LIGHT_UP;
byte nextLED = 0;
byte playingAlbum;
File album;
int volume = 25;
boolean headphone = false;
boolean headphoneFirstMeasure = false;


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
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, HIGH);   // LED off
  digitalWrite(GREEN_LED_PIN, HIGH); // LED off
  digitalWrite(BLUE_LED_PIN, HIGH);  // LED off
}

void initializeAmplifier() {
  pinMode(AMPLIFIER_ENABLE_PIN, OUTPUT);
  pinMode(HEADPHONE_LEVEL_PIN, INPUT_PULLUP);
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
  pinMode(TRELLIS_INT_PIN, INPUT_PULLUP);

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

  // Blink in pause mode
  } else if (state == PLAY_PAUSED) {
    blinkLED(BLUE_LED_PIN);
    return PAUSE_DELAY;
  }
}

void tickIdleTimeout() {
  Serial.println("Timeout!");
  onTimeout();

  attachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN), trellisIsr, LOW);
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
  blinkLED(BLUE_LED_PIN);
}

void blinkLED(int pin) {
  digitalWrite(pin, LOW); // LED on
  delay(1);
  digitalWrite(pin, HIGH); // LED off
}

void trellisIsr() {
  detachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN));
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

  // Read encoder switch
  static bool consumedHeld = false;
  ClickEncoder::Button button = encoder.getButton();
  if (button == ClickEncoder::Clicked) {
    switch (state) {
      case PLAY_SELECTED: onPause(true); break;
      case PLAY_PAUSED:   onPause(false); break;
    }
  } else if (button == ClickEncoder::Held && !consumedHeld) {
    consumedHeld = true;
    switch (state) {
      case PLAY_SELECTED:
      case PLAY_PAUSED:
        Serial.print("Stopped #"); Serial.println(playingAlbum);
        blinkLED(BLUE_LED_PIN);
        stopPlaying();
        onStopPlaying();
      break;

      case IDLE_LIGHT_UP:
      case IDLE_TURN_OFF:
      case IDLE_WAIT_OFF:
        Serial.println("Force timeout");
        blinkLED(BLUE_LED_PIN);
        delay(300);
        blinkLED(BLUE_LED_PIN);
        nextTimeoutTick = millis();
      break;
    }
  } else if (button == ClickEncoder::Released) {
    consumedHeld = false;
  }

  // Read headphone level
  int audioLevel = analogRead(HEADPHONE_LEVEL_PIN);
  if (headphone != headphoneFirstMeasure && (audioLevel > HEADPHONE_THRESHOLD) != headphoneFirstMeasure) {
    // confirmed change
    onHeadphoneInserted(audioLevel < HEADPHONE_THRESHOLD);
    Serial.print("Confirmed change, headphone "); Serial.println(headphoneFirstMeasure);
  } else if ((audioLevel < HEADPHONE_THRESHOLD) != headphoneFirstMeasure) {
    // there seems to be a change
    headphoneFirstMeasure = (audioLevel < HEADPHONE_THRESHOLD);
    Serial.print("Possible change, headphone "); Serial.println(headphoneFirstMeasure);
  }

  return READ_DELAY;
}

void onKey(byte index) {
  // No keys when paused
  if (state == PLAY_PAUSED) {
    // nop
    Serial.println("Still paused");

  // Same key pressed again
  } else if (state == PLAY_SELECTED && playingAlbum == index) {
    nextTimeoutTick = 0; // no timeout during playing
    stopPlaying();
    onTryNextTrack();

  // Some (other) key pressed
  } else {
    if (state == PLAY_SELECTED) stopPlaying();
    nextTimeoutTick = 0; // no timeout during playing
    enablePlayer(true);
    enableAmplifier(!headphone);
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
    Serial.print("Ended album #"); Serial.println(playingAlbum);
    onStopPlaying();
  }
}

void onPause(bool pause) {
  if (pause) {
    Serial.println("Pause");
    trellis.blinkRate(HT16K33_BLINK_HALFHZ);
    player.pausePlaying(true);
    enableAmplifier(false);
    nextTimeoutTick = millis() + PAUSE_TIMEOUT;
    state = PLAY_PAUSED;
  } else {
    Serial.println("Resume");
    trellis.blinkRate(HT16K33_BLINK_1HZ);
    enableAmplifier(!headphone);
    player.pausePlaying(false);
    nextTimeoutTick = 0; // no timeout during playing
    state = PLAY_SELECTED;
  }
}

void onStopPlaying() {
  enableAmplifier(false);
  enablePlayer(false);
  if (album && album.isDirectory()) album.close();
  onEnterIdle(1200);
}

void onHeadphoneInserted(boolean plugged) {
  headphone = plugged;
  if (plugged) {
    volume = volume + HEADPHONE_VOLUME_MAX - SPEAKER_VOLUME_MAX;
  } else {
    volume = volume + SPEAKER_VOLUME_MAX - HEADPHONE_VOLUME_MAX;
  }
  enableAmplifier(!headphone);
}

// Set volume for left, right channels. lower numbers == louder volume!
//   0: max volume
//  20: cracks on amplified loudspeaker
// 130: audible
// 254: min volume
// 255: analog off
void changeVolume(int encoderChange) {
  volume += encoderChange;
  if (headphone) {
    volume = max(HEADPHONE_VOLUME_MAX, min(volume, HEADPHONE_VOLUME_MIN));
  } else {
    volume = max(SPEAKER_VOLUME_MAX, min(volume, SPEAKER_VOLUME_MIN));
  }
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

