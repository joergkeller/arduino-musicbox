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

#define ENCODER_A_PIN       A0
#define ENCODER_B_PIN       A1
#define VOLUME_DIRECTION    -3    // set direction of encoder-volume
#define ENCODER_SWITCH_PIN   0

#define RED_LED_PIN         A3
#define GREEN_LED_PIN       A4
#define BLUE_LED_PIN        A5

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
        OneButton button = OneButton(ENCODER_SWITCH_PIN, LOW, false);

        CallbackIntFunction rotaryFunction;

        void blinkLED(int pin);
};

#endif
