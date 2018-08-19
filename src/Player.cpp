/*
 * Class to read music files from SD card, decode it in the MusicMaker 
 * and amplify the sound unless a headphone is plugged in.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "Player.h"


Player::Player() {
  pinMode(AMPLIFIER_ENABLE_PIN, OUTPUT);
  pinMode(MUSIC_RESET_PIN, OUTPUT);
  pinMode(HEADPHONE_LEVEL_PIN, INPUT_PULLUP);
}

void Player::initialize() {
  initializeAmplifier();
  initializeCard();
  initializePlayer();
}

void Player::initializeAmplifier() {
  enableAmplifier(false);
}

void Player::initializeCard() {
  if (!SD.begin(CARD_CS_PIN)) {
    Serial.println(F("SD failed or not inserted"));
    return;  // don't do anything more
  }
  Serial.println(F("SD initialized"));

//  File root = SD.open(F("/"));
//  printDirectory(root, 0);
//  root.close();
}

void Player::printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) break;
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println(F("/"));
      printDirectory(entry, numTabs+1);
    } else {
      Serial.print(F("\t\t"));
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void Player::initializePlayer() {
  digitalWrite(MUSIC_RESET_PIN, HIGH);
  if (!player.begin()) {
    Serial.println(F("VS1053 failed"));
    return;
  }
  player.softReset();
  //player.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  player.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
  Serial.println(F("VS1053 initialized"));

  enablePlayer(false);
}

void Player::enable(bool enable) {
  enablePlayer(enable);
  enableAmplifier(enable && !headphone);
}

void Player::enablePlayer(bool enable) {
  if (enable) {
    digitalWrite(MUSIC_RESET_PIN, HIGH);
    player.reset();
    player.setVolume(volume, volume);
  } else {
    player.setVolume(VOLUME_OFF, VOLUME_OFF);
    digitalWrite(MUSIC_RESET_PIN, LOW);
  }
}

void Player::enableAmplifier(bool enable) {
  digitalWrite(AMPLIFIER_ENABLE_PIN, enable);
}

/*
 * Set volume for left, right channels. lower numbers == louder volume!
 *   0: max volume
 *  ~5: cracks on amplified loudspeaker
 * 130: audible
 * 254: min volume
 * 255: analog off
 */
void Player::changeVolume(int encoderChange) {
  volume += encoderChange;
  if (headphone) {
    volume = max(HEADPHONE_VOLUME_MAX, min(volume, HEADPHONE_VOLUME_MIN));
  } else {
    volume = max(SPEAKER_VOLUME_MAX, min(volume, SPEAKER_VOLUME_MIN));
  }
  Serial.print(F("Set Volume ")); Serial.println(volume);
  player.setVolume(volume, volume);
}

/*
 * Read headphone level.
 * An inserted headphone plug will tear the dc level down. The actual level depends on the headphone used.
 */
void Player::checkHeadphoneLevel() {
  int audioLevel = analogRead(HEADPHONE_LEVEL_PIN);
  if (headphone != headphoneFirstMeasure && (audioLevel > HEADPHONE_THRESHOLD) != headphoneFirstMeasure) {
    // confirmed change
    onHeadphoneInserted(audioLevel < HEADPHONE_THRESHOLD);
    Serial.print(F("Confirmed change, headphone ")); Serial.println(headphoneFirstMeasure);
  } else if ((audioLevel < HEADPHONE_THRESHOLD) != headphoneFirstMeasure) {
    // there seems to be a change
    headphoneFirstMeasure = (audioLevel < HEADPHONE_THRESHOLD);
    Serial.print(F("Possible change, headphone ")); Serial.println(headphoneFirstMeasure);
  }
}

void Player::onHeadphoneInserted(bool plugged) {
  headphone = plugged;
  if (plugged) {
    volume = volume + HEADPHONE_VOLUME_MAX - SPEAKER_VOLUME_MAX;
  } else {
    volume = volume + SPEAKER_VOLUME_MAX - HEADPHONE_VOLUME_MAX;
  }
  enableAmplifier(!headphone);
  Serial.print(F("Set Volume ")); Serial.println(volume);
  player.setVolume(volume, volume);
}

bool Player::startFirstTrack(byte albumIndex) {
  // close old album (if any)
  if (album && album.isDirectory()) { album.close(); }
  // open indexed album
  int albumNr = albumIndex + 1;
  String albumTemplate = F("ALBUM");
  String albumName = albumTemplate + albumNr;
  album = SD.open(albumName);
  return nextTrack();
}

bool Player::startFile(char* trackName) {
  if (album && album.isDirectory()) { album.close(); }
  return player.startPlayingFile(trackName);
}

bool Player::nextTrack() {
  if (!album || !album.isDirectory()) return false;
  File track = album.openNextFile();
  if (!track) return false;

  String albumName = album.name();
  String dirPath = albumName + '/';
  String filePath = dirPath + track.name();
  Serial.print(F("Next track: ")); Serial.println(filePath);
  track.close();

  player.startPlayingFile(filePath.c_str());
  return true;
}

void Player::pause(bool pause) {
  player.pausePlaying(pause);
  enableAmplifier(!pause && !headphone);
}

void Player::stop() {
  player.stopPlaying();
  delay(20);
}

bool Player::hasStopped() {
  return player.stopped();
}
