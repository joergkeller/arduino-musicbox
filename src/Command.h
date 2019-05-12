/*
 * Class to read a rotary encoder with button for volume control and pause/play.
 *
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef Command_h
#define Command_h

#include <RotaryEncoder.h>
#include <OneButton.h>

#define ENCODER_A_PIN       A0    // (AVR A0/#18 -> ESP32 A0/#26)
#define ENCODER_B_PIN       A1    // (AVR A1/#19 -> ESP32 A1/#25)
#define ENCODER_SWITCH_PIN  A2    // (moved to Pin 7, ESP32 A2/#34)
#define VOLUME_DIRECTION    -3    // set direction of encoder-volume

#define RGB_LED_PIN         A4    // (moved to Pin 9, ESP32 A4/#36)

#define COLOR_RED         1
#define COLOR_GREEN       2
#define COLOR_BLUE        3
#define BLINK_DURATION   10 // ms

extern "C" {
  typedef void (*CallbackIntFunction)(int);
  typedef void (*CallbackVoidFunction)(void);
}

class Command {
    public:
        Command();
        void initialize();
    
        void tickMs();

        void attachRotary(CallbackIntFunction callback) {
            rotaryFunction = callback;
        }
        void attachClick(CallbackVoidFunction clickHandler) {
            button.attachClick((callbackFunction)clickHandler);
        }
        void attachDoubleClick(CallbackVoidFunction doubleclickHandler) {
            button.attachDoubleClick((callbackFunction)doubleclickHandler);
        }
        void attachLongClick(CallbackVoidFunction holdHandler) {
            button.attachLongPressStart((callbackFunction)holdHandler);
        }

        void blink(int color);

    private:
        RotaryEncoder encoder = RotaryEncoder(ENCODER_A_PIN, ENCODER_B_PIN);
        OneButton button = OneButton(ENCODER_SWITCH_PIN, HIGH, true);

        CallbackIntFunction rotaryFunction;

        void blinkLED(int pin);
};

#endif
