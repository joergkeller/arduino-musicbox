# Arduino MusicBox

Arduino control code for a music maker board and a trellis keypad.

Hardware parts:
* Adafruit Feather 32u4 Basic Proto
* Adafruit FeatherWing MusicMaker
* Adafruit Trellis PCB, Elastomer, LED
* Amplifier
* Rotary Decoder
* LiPolymer 3.7V Akku

The control code shall
* read trellis keys
* on key press play a music file from the micro SD card from a specific folder and blink the respective key
* on pressing the same key or when playback stops jump to the next file in the folder, until all music is played
* read the rotary decoder to change the volume, the maximum volume can be configured
* detect when a headphone is plugged in and mute the amplifier for the internal speakers
* switch the music box  off automatically after an idle period