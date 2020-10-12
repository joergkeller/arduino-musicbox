/*
 * Class to read music files from SD card, decode it in the MusicMaker 
 * and amplify the sound unless a headphone is plugged in.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef Player_h
#define Player_h

#include <Arduino.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// Feather/Wing pin setup
#define MUSIC_RESET_PIN   12     // VS1053 reset pin
#define CARD_CS_PIN        5     // Card chip select pin
#define MUSIC_CS_PIN       6     // VS1053 chip select pin (output)
#define MUSIC_DCS_PIN     10     // VS1053 Data/command select pin (output)
#define MUSIC_DREQ_PIN     9     // VS1053 Data request, ideally an Interrupt pin (not possible on 32u4)

// Amplifier pin setup
#define AMPLIFIER_ENABLE_PIN   11   // Enable both amplifier channels
#define HEADPHONE_LEVEL_PIN    A2   // Voltage level indicates headphone plugin
#define HEADPHONE_THRESHOLD   100   // Plugged-in: ~20, Unplugged: ~890

// Volume
#define VOLUME_INITIAL              40    // moderation level: smaller number = louder
#define SPEAKER_VOLUME_MIN         100    // max. 254 moderation
#define SPEAKER_VOLUME_MAX           5    // min. 0 moderation
#define HEADPHONE_VOLUME_MIN       100    // max. 254 moderation
#define HEADPHONE_VOLUME_MAX        15    // min. 0 moderation
#define VOLUME_OFF                 255    // 255 = switch audio off, TODO avoiding cracking noise, maybe correct stuffing needed when stop


class Player {
	
  public:
    Player();
    void initialize();
	
    void enable(bool enable);
    void changeVolume(int encoderChange);
    void checkHeadphoneLevel();

    bool startPlaying(const char* path);
    bool nextTrack();
    void pause(bool pause);
    void stop();
    bool hasStopped();
	
  private:
    Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET_PIN, MUSIC_CS_PIN, MUSIC_DCS_PIN, MUSIC_DREQ_PIN, CARD_CS_PIN);

    String albumPath = String();
    File album;
    int volume = VOLUME_INITIAL;
    bool headphone = false;
    bool headphoneFirstMeasure = false;
	
    void initializeAmplifier();
    void initializeCard();
    void printDirectory(File dir, int numTabs);
    void initializePlayer();
    void enablePlayer(bool enable);
    void enableAmplifier(bool enable);
    void onHeadphoneInserted(bool plugged);
    String extendPath(String path, String fileName);
};

#endif
