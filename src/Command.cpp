/*
 * Class to read a rotary encoder with button for volume control and pause/play.
 *
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "Command.h"


Command::Command() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
}

void Command::initialize() {
  digitalWrite(RED_LED_PIN, HIGH);   // LED off
  digitalWrite(GREEN_LED_PIN, HIGH); // LED off
  digitalWrite(BLUE_LED_PIN, HIGH);  // LED off
}

void Command::tickMs() {
  static int pos = 0;
  encoder.tick();
  button.tick();

  int newPos = encoder.getPosition();
  if (pos != newPos) {
    int delta = (newPos - pos) * VOLUME_DIRECTION;
    if (rotaryFunction) { rotaryFunction(delta); }
    pos = newPos;
  }
}

void Command::blink(int color) {
  switch(color) {
    case COLOR_RED:   blinkLED(RED_LED_PIN); break;
    case COLOR_GREEN: blinkLED(GREEN_LED_PIN); break;
    case COLOR_BLUE:  blinkLED(BLUE_LED_PIN); break;
  }
}

void Command::blinkLED(int pin) {
  digitalWrite(pin, LOW); // LED on
  delay(BLINK_DURATION);
  digitalWrite(pin, HIGH); // LED off
}
