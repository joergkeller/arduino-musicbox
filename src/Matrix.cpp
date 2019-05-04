/*
 * Class to drive a trellis display matrix.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */

#include "Matrix.h"

Matrix::Matrix() {
  // INT pin requires a pullup;
  // this is also the MIDI Tx pin recommended to bound to high
  pinMode(TRELLIS_INT_PIN, INPUT_PULLUP);
}

void Matrix::initialize() {
  // clear display
  trellis.begin();
  trellis.pixels.clear();
  trellis.pixels.show();
}

void Matrix::tickMs() {
  if (++readTicks > KEY_READ_DELAY) {
    readTicks = 0;
    trellis.read(true); // polling mode
  }
  if (isIdle) {
    show.tickMs();
  }
}

void Matrix::idle() {
  isIdle = true;
  show.initialize();
}

void Matrix::blink(byte index, bool fast) {
  isIdle = false;
  trellis.pixels.clear();
  trellis.pixels.setPixelColor(index, show.wheel(index, BRIGHTNESS_PLAYING));
  trellis.pixels.show();
}

void Matrix::waitForNoKeyPressed() {
}

int Matrix::getPressedKey() {
  return -1;
}

/*
 * Activate all keys and set callback.
 */
void Matrix::attachKeyPress( TrellisCallback (*handler)(keyEvent)) {
  for (int i = 0; i < NUMKEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    //trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, handler);
  }
}

void Matrix::sleep() {
  isIdle = false;
  /*
  trellis.clear();
  trellis.writeDisplay();
  trellis.sleep();
  */
}

void Matrix::wakeup() {
  isIdle = false;
  /*
  trellis.wakeup();
  */
}

void Matrix::enableInterrupt(void (*isr)(void)) {
  attachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN), isr, LOW);
}

void Matrix::disableInterrupt() {
  detachInterrupt(digitalPinToInterrupt(TRELLIS_INT_PIN));
}
