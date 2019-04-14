/*
 * Class to control the lightshow of a trellis display.
 * 
 * Written by Jörg Keller, Winterthur, Switzerland
 * https://github.com/joergkeller/arduino-musicbox
 * MIT license, all text above must be included in any redistribution
 */
#ifndef ShowAbstract_h
#define ShowAbstract_h

#include <Arduino.h>

// Trellis setup
#define NUMKEYS   16


class ShowAbstract {
  public:
    virtual void initialize() = 0;
    virtual void tickMs() = 0;

  protected:
    /*
     * Input a value 0 to 255 to get a color value.
     * The colors are a transition r - g - b - back to r.
     */
    uint32_t wheel(byte index) {
      byte maxKey = NUMKEYS - 1;
      byte pos = map(index, 0, maxKey, 0, 255);
      if (pos < 85) {
       return color(pos * 3, 255 - pos * 3, 0);
      } else if (pos < 170) {
       pos -= 85;
       return color(255 - pos * 3, 0, pos * 3);
      } else {
       pos -= 170;
       return color(0, pos * 3, 255 - pos * 3);
      }
      return 0;
    };

    /*
     * Same as above but with an additional brightness 0..15
     */
    uint32_t wheel(byte index, uint8_t brightness) {
      byte maxKey = NUMKEYS - 1;
      byte pos = map(index, 0, maxKey, 0, 255);
      if (pos < 85) {
       return color(pos * 3, 255 - pos * 3, 0, brightness);
      } else if (pos < 170) {
       pos -= 85;
       return color(255 - pos * 3, 0, pos * 3, brightness);
      } else {
       pos -= 170;
       return color(0, pos * 3, 255 - pos * 3, brightness);
      }
      return 0;
    };

    /*
     * Convert separate R,G,B into packed 32-bit RGB color.
     * Packed format is always RGB, regardless of LED strand color order.
     */
    uint32_t color(uint32_t r, uint32_t g, uint32_t b) {
      return (r << 16) | (g <<  8) | b;
    }

    /* 
     * Same as above but with an additional factor (0..255) to adjust brightness.
     */
    uint32_t color(uint32_t r, uint32_t g, uint32_t b, uint8_t f) {
      return color((r * f) >> 8, (g * f) >> 8, (b * f) >> 8);
    }
};

#endif
