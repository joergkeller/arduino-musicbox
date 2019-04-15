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
  friend class Matrix;
  
  public:
    virtual void initialize() = 0;
    virtual void tickMs() = 0;

  protected:
    /*
     * Input the index of a key to get a color value.
     * The colors are a transition r - g - b - back to r.
     */
    uint32_t wheel(byte index, uint32_t brightness) {
      byte maxKey = NUMKEYS - 1;
      uint32_t pos = map(index, 0, maxKey, 0, 767);
      uint8_t phase = pos / 256;    // 0..2
      uint8_t offset = pos % 256;
      if (phase == 0) {
        return color(255 - offset, offset, 0, brightness + 1);
      } else if (phase == 1) {
        return color(0, 255 - offset, offset, brightness + 1);
      } else {
        return color(offset, 0, 255 - offset, brightness + 1);
      }
    };

    /*
     * Convert separate R,G,B into packed 32-bit RGB color.
     * Packed format is always RGB, regardless of LED strand color order.
     */
    uint32_t color(uint32_t r, uint32_t g, uint32_t b) {
      return (r << 16) | (g << 8) | b;
    }

    /* 
     * Same as above but with an additional factor (0..256) to adjust brightness.
     */
    uint32_t color(uint32_t r, uint32_t g, uint32_t b, uint32_t f) {
      return color((r * f) >> 8, (g * f) >> 8, (b * f) >> 8);
    }
};

#endif
