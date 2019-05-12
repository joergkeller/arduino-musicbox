/***************************************************
 * 
 ***************************************************/

#include "Player.h"

#define IDLE           0
#define DIRECT_PLAYING 1
#define ALBUM_PLAYING  2

Player player = Player();
byte state = IDLE;
unsigned long tickMs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("MusicMaker Player Test");

  player.initialize();
  player.enable(true);
  //Serial.println(F("Playing track Tom"));
  //player.startFile("/album7/Tom.mp3");
  state = DIRECT_PLAYING;
}

void loop() {
  unsigned long now = millis();
  if (now != tickMs) {
    tickMs = now;
    player.tickMs();
    if (player.hasStopped()) {
      onPlayerStopped();
    }
  }
}

void onPlayerStopped() {
  if (state == DIRECT_PLAYING) {
    Serial.println(F("Playing album 15"));
    player.startFirstTrack(15);
    state = ALBUM_PLAYING;
  } else if (state == ALBUM_PLAYING) {
    Serial.println(F("Playing next track"));
    if (!player.nextTrack()) {
      Serial.println(F("Stop playing album"));
      state = IDLE;
    }
  }
}
