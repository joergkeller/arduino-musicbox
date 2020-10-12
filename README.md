# Arduino MusicBox

Arduino control code for a music maker board and a trellis keypad with optional nfc reader.

Hardware parts:
* Adafruit Feather 32u4 Basic Proto
* Adafruit FeatherWing MusicMaker
* Adafruit Trellis PCB, Elastomer, LED
* PN532 NFC reader
* Amplifier, Stereo speaker 
* Rotary Decoder
* LiPolymer 3.7V Akku

The control code shall
* animate the trellis keys while idle
* read trellis keys
* on key press play a music track from the micro SD card from a configured folder and blink the respective key
* on pressing the same key or when playback stops jump to the next track in the folder, until all music is played
* read the rotary decoder to change the volume, the maximum volume can be configured
* press the rotary decoder to pause/resume (short) and stop/sleep (long)
* detect when a headphone is plugged in and mute the amplifier for the internal speakers
* read nfc chips and play the configured track or album for the given id
* switch the music box  off automatically after an idle period

## Checkout this code
```
git clone https://github.com/joergkeller/arduino-musicbox.git
cd arduino-musicbox
git submodule init
git submodule update
```

## Remarks on the hardware
* See [wiring schema](extras/schema/MusicBox_Schaltplan.pdf)
* Cut the "RESET" bridge on the Adafruit MusicMaker FeatherWing (otherwise a reset of the mp3 chip will also 
  reset the arduino board).
* Set dip switches on the NFC board to use I2C (1: ON, 2: OFF)

## Prepare micro SD card
Copy \<musicbox\>/extras/config/*.cfg in root directory of the SD card.\
Copy music files to the SD card, using 8.3 filenames without special characters.
Assign paths to buttons in buttons.cfg and NFC ids in nfc.cfg.\
Paths can be a folder (all tracks will be played) or a specific story/song. Music files can be in subfolders, but
deeply nested structures should be avoided.


## Howto install using Arduino IDE

File > Preferences > Sketchbook locations:\
\<musicbox\> folder containing src, libraries, extra folders

File > Preferences > Additional Boards Manager URLs:\
https://adafruit.github.io/arduino-board-index/package_adafruit_index.json

Tools > Board > Boards Manager\
 Install Adafruit AVR Boards

Restart Arduino IDE\
Open \<musicbox\>/src/src.ino

Tools > Boards\
 Adafruit Feather 32u4

Tools > Ports\
\<USB port connected to the feather board\>

> Verify/Compile\
> Upload

## Howto install using Arduino CLI
```
arduino-cli core update-index
arduino-cli core install arduino:avr@1.8.3
arduino-cli core install adafruit:avr@1.4.13
arduino-cli compile src --fqbn=adafruit:avr:feather32u4
arduino-cli board list
arduino-cli upload src --fqbn=adafruit:avr:feather32u4 --port COM4 --verify --verbose
```
