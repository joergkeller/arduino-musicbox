/***************************************************
  Arduino control code for a music maker board and a trellis keypad with optional nfc reader.

  The control code shall
     animate the trellis keys while idle (/)
     read trellis keys (/)
     on key press play a music file from the micro SD card from a specific folder and blink the respective key (/)
     on pressing the same key or when playback stops jump to the next file in the folder, until all music of that folder is played (/)
     read the rotary decoder to change the volume, the maximum volume can be configured (/)
     press the rotary decoder to pause/resume (short) and stop/sleep (long) (/)
     detect when a headphone is plugged in and mute the amplifier for the internal speakers (/)
     read nfc chips and play the configured file for the given id (/)
     switch the music box off automatically (sleep mode) after an idle period, wake on key press (/)

  Todos:
   - Audio off when idle (volume 255,255) -> proper file padding needed (?)

  Written by Jörg Keller, Switzerland
  https://github.com/joergkeller/arduino-musicbox
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Properties.h>
#include "Command.h"
#include "Matrix.h"
#include "Player.h"

#ifndef ESP32
#include <LowPower.h>
#endif

// Delays [ms]
#define PAUSE_DELAY    1000
#define READ_DELAY       50
#define NFC_DELAY      1000
#define IDLE_TIMEOUT  (1000L * 60L * 15L)
#define PAUSE_TIMEOUT (1000L * 60L * 60L)
#define ROLLOVER_GAP  (1000L * 60L * 60L)

// NFC pin setup
#define NFC_RESET_PIN   13    // PN532 reset pin

// States
#define IDLE            1
#define PLAY_SELECTED   2
#define PLAY_PAUSED     3
#define TIMEOUT_WAIT    4 

// NFC reader
#define NFC_RETRIES   0    // 0 = one try, 255 = retry forever

/***************************************************
   Variables
 ****************************************************/
Command command = Command();
Matrix matrix = Matrix();
Player player = Player();
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);


unsigned long tickMs = 0;
unsigned long nextReadTick = millis() + 1;
unsigned long nextNfcTick = millis() + 1;
unsigned long nextIdleTick = millis() + 1;
unsigned long nextTimeoutTick = 0;
byte state = IDLE;
byte playingAlbum;

/***************************************************
   Setup
 ****************************************************/
void setup() {
  Serial.begin(19200);
  while (!Serial && millis() < nextIdleTick + 75);
  Serial.println(F("MusicBox setup"));

  command.initialize();
  matrix.initialize();
  player.initialize();
  initializeNfc();

  command.attachRotary(onRotaryChange);
  command.attachClick(onClick);
  command.attachLongClick(onLongClick);
  onEnterIdle(0);
}

void initializeNfc() {
  pinMode(NFC_RESET_PIN, OUTPUT);
  digitalWrite(NFC_RESET_PIN, HIGH);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println(F("PN532 failed"));
    return;
  }

  // Set the max number of retry attempts to read from a card.
  // This prevents us from waiting forever for a card, which is the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(NFC_RETRIES);
  nfc.SAMConfig();  
  Serial.println(F("PN532 initialized"));
}

void onEnterIdle(unsigned int delay) {
  state = IDLE;
  matrix.idle();
  enableNfc(true);

  nextReadTick = millis() + 1;
  nextNfcTick = millis() + 1;
  nextIdleTick = millis() + 1 + delay;
  nextTimeoutTick = millis() + IDLE_TIMEOUT;
}

/***************************************************
   External event handler
 ****************************************************/
void onRotaryChange(int delta) {
  player.changeVolume(delta);
}

void onClick() {
  switch (state) {
    case PLAY_SELECTED: onPause(true); break;
    case PLAY_PAUSED:   onPause(false); break;
  }
}

void onLongClick() {
  switch (state) {
    case PLAY_SELECTED:
    case PLAY_PAUSED:
      Serial.print(F("Stopped #")); Serial.println(playingAlbum);
      command.blink(COLOR_RED);
      player.stop();
      onStopPlaying();
    break;

    case IDLE:
      Serial.println(F("Force timeout"));
      command.blink(COLOR_GREEN);
      delay(300);
      command.blink(COLOR_RED);
      nextTimeoutTick = millis();
    break;
  }
}

/***************************************************
   Loop
 ****************************************************/
void loop() {
  unsigned long now = millis();

  // Rollover millis since start
  if (nextReadTick > now + ROLLOVER_GAP) {
    Serial.println(F("Rollover timer ticks!"));
    nextReadTick = now + 1;
    nextIdleTick = now + 1;
    nextTimeoutTick = nextTimeoutTick == 0 ? 0 : now + IDLE_TIMEOUT;
  }

  // Next ticks (timeout check first to go back to sleep mode after blink)
  if (0 < nextTimeoutTick && nextTimeoutTick <= now) {
    tickIdleTimeout(now);
  }
  if (0 < nextReadTick && nextReadTick <= now) {
    tickReadKeys(now);
  }
  if (0 < nextNfcTick && nextNfcTick <= now) {
    tickReadNfc(now);
  }
  if (0 < nextIdleTick && nextIdleTick <= now) {
    tickIdleShow(now);
  }

  if (now != tickMs) {
    command.tickMs();
    matrix.tickMs();
    player.checkHeadphoneLevel();
    tickMs = now;
  }

  // Check for finished track
  if (state == PLAY_SELECTED) {
    if (player.hasStopped()) onTryNextTrack();
  }
}

void tickIdleShow(unsigned long now) {
  // Blink in pause mode
  if (state == PLAY_PAUSED) {
    command.blink(COLOR_BLUE);
    nextIdleTick = now + PAUSE_DELAY;
  }
}

void tickIdleTimeout(unsigned long now) {
  Serial.println(F("Timeout!"));
  onTimeout();

  matrix.enableInterrupt(trellisIsr);
  #if defined (__AVR__)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  #endif
  #if defined (__arm__)
    // TODO power down M0
    LowPower.standby();
  #endif
  #if defined (ESP32)
    // TODO power down ESP32
  #endif

  if (state == TIMEOUT_WAIT) {
    // temporary wakeup after 8s, no interrupt called
    onWatchdogPing();
  } else {
    onSleepWake();
  }
}

void onTimeout() {
  matrix.sleep();
  enableNfc(false);
  player.enable(false);
  
  state = TIMEOUT_WAIT;
}

void onWatchdogPing() {
  command.blink(COLOR_BLUE);
}

void trellisIsr() {
  matrix.disableInterrupt();
  state = IDLE;
}

void onSleepWake() {
  matrix.wakeup();
  // ignore wakeup key
  matrix.waitForNoKeyPressed();
  onEnterIdle(0);
}

void tickReadKeys(unsigned long now) {
  // Read trellis keys
  int index = matrix.getPressedKey();
  if (index != -1) {
    onKey(index);
  }

  nextReadTick = now + READ_DELAY;
}

unsigned long tickReadNfc(unsigned long now) {
  // Wait for an ISO14443A type cards (Mifare, etc.).
  // Length of uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight).
  byte uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  byte uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)) {
    String hexId = String("");
    for (uint8_t i=0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        hexId += '0';
      }
      hexId += String(uid[i], HEX);
    }
    hexId.toUpperCase();
    Serial.print(F("NFC UID: 0x")); Serial.println(hexId);
    onNfcId(hexId);
  } else {
    // No NFC card found, try again
    nextNfcTick = now + NFC_DELAY;
  }
}

void onKey(byte index) {
  // No keys when paused
  if (state == PLAY_PAUSED) {
    // nop
    Serial.println(F("Still paused"));

  // Same key pressed again
  } else if (state == PLAY_SELECTED && playingAlbum == index) {
    nextTimeoutTick = 0; // no timeout during playing
    nextNfcTick = 0; // no nfc reading during playing
    player.stop();
    onTryNextTrack();

  // Some (other) key pressed
  } else {
    if (state == PLAY_SELECTED) player.stop();
	  matrix.blink(index, true);
    nextTimeoutTick = 0; // no timeout during playing
    nextNfcTick = 0; // no nfc reading during playing
    enableNfc(false);
	  player.enable(true);
	  if (player.startFirstTrack(index)) {
      state = PLAY_SELECTED;
	    playingAlbum = index;
      Serial.print(F("Playing album #")); Serial.println(playingAlbum);
    } else {
      Serial.print(F("Failed album #")); Serial.println(playingAlbum);
      onStopPlaying();
    }
  }
}

void onNfcId(String hexId) {
  if (state == PLAY_SELECTED || state == PLAY_PAUSED) {
    return;
  }

  File file = SD.open("nfc.cfg");
  Properties cfg = Properties(file);
  String trackName = cfg.readString(hexId);
  file.close();

  if (trackName.length() == 0) {
    Serial.print(F("Unknown nfc id ")); Serial.println(hexId);
    File file = SD.open("nfc.cfg", FILE_WRITE);
    file.write((unsigned char*)hexId.c_str(), hexId.length());
    file.write((unsigned char*)"=\n", 2);
    file.close();
  } else {
    Serial.print(F("Playing file ")); Serial.println(trackName);
    onNfcPlay(trackName);
  }
}

void onNfcPlay(String trackName) {
  state = PLAY_SELECTED;
  playingAlbum = 0;
  matrix.blink(playingAlbum, true);
  nextTimeoutTick = 0; // no timeout during playing
  nextNfcTick = 0; // no nfc reading during playing
  enableNfc(false);
  player.enable(true);
  player.startFile((char*)trackName.c_str());
}

void onTryNextTrack() {
  if (player.nextTrack()) {
    matrix.blink(playingAlbum, true);
    state = PLAY_SELECTED;
  } else {
    Serial.print(F("Ended album #")); Serial.println(playingAlbum);
    onStopPlaying();
  }
}

void onPause(bool pause) {
  if (pause) {
    Serial.println(F("Pause"));
    matrix.blink(playingAlbum, false);
    player.pause(true);
    nextTimeoutTick = millis() + PAUSE_TIMEOUT;
    state = PLAY_PAUSED;
  } else {
    Serial.println(F("Resume"));
    matrix.blink(playingAlbum, true);
    player.pause(false);
    nextTimeoutTick = 0; // no timeout during playing
    state = PLAY_SELECTED;
  }
}

void onStopPlaying() {
  player.enable(false);
  onEnterIdle(1200);
}

void enableNfc(bool enable) {
  pinMode(NFC_RESET_PIN, OUTPUT);
  if (enable) {
    Serial.println("Enable NFC");
    digitalWrite(NFC_RESET_PIN, HIGH);
    initializeNfc();
  } else {
    Serial.println("Disable NFC");
    digitalWrite(NFC_RESET_PIN, LOW);    
  }
}
