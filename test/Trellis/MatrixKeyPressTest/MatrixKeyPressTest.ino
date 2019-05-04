/*
 * Show usage of the Matrix helper class wrapping a NeoTrellis.
 * Copy this test into the src.ino to compile it using the helper class.
 */

#include <Wire.h>
#include "Matrix.h"

Matrix matrix = Matrix();
unsigned long tickMs = 0;

void setup() {
  Serial.begin(19200);
  Serial.println("Starting trellis idle show with key presses");

  matrix.initialize();
  matrix.attachKeyPress(blink);
  matrix.idle();
}

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  matrix.blink(evt.bit.NUM, false);
}

void loop() {
  unsigned long now = millis();
  if (now != tickMs) {
    matrix.tickMs();
    tickMs = now;
  }
}
