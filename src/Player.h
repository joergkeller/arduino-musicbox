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
#define MUSIC_RESET_PIN   12     // VS1053 reset pin (AVR/ESP32 both #12)
#define CARD_CS_PIN       14     // Card chip select pin (output) (AVR #5 -> ESP32 #14)
#define MUSIC_CS_PIN      32     // VS1053 chip select pin (output) (AVR #6 -> ESP32 #32)
#define MUSIC_DCS_PIN     33     // VS1053 Data/command select pin (output) (AVR #10 -> ESP32 #33)
#define MUSIC_DREQ_PIN    15     // VS1053 Data request, ideally an Interrupt pin (AVR #9 -> ESP32 #15)

// Amplifier pin setup
#define AMPLIFIER_ENABLE_PIN   27   // Enable both amplifier channels (AVR #11 -> ESP32 #27)
#define HEADPHONE_LEVEL_PIN    A3   // Voltage level indicates headphone plugin (moved to Pin 8, ESP32 A3/#39)
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

    void tickMs();
	
    void enable(bool enable);
    void changeVolume(int encoderChange);
    void checkHeadphoneLevel();
  
    bool startFirstTrack(byte albumIndex);
    bool startFile(char* trackName);
    bool nextTrack();
    void pause(bool pause);
    void stop();
    bool hasStopped();
	
  private:
    Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(MUSIC_RESET_PIN, MUSIC_CS_PIN, MUSIC_DCS_PIN, MUSIC_DREQ_PIN, CARD_CS_PIN);
    
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
};

#endif
