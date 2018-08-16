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


class ShowAbstract {
  public:
    virtual void initialize() = 0;
    virtual void tickMs() = 0;
};

#endif
