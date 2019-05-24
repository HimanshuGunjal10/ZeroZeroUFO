/*
 * RGB.hpp
 *
 *  Created on: Nov 14, 2018
 *      Author: neelpatel
 */

#ifndef RGB_HPP_
#define RGB_HPP_


#include "GFX.hpp"

class RGB : public GFX {

public:

    static RGB& getRGBInstance();
    static RGB* rgbInstance;

    void
       begin(void);
    bool drawPixel(uint16_t x, uint16_t y, uint16_t c);
    void fillScreen(uint16_t c),
       updateDisplay(void);
    void drawObstacle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c, bool isRectangle);

    void init(uint8_t rows, uint32_t a, uint32_t b, uint32_t c,uint32_t d , uint32_t e,
       uint8_t sclk, uint32_t latch, uint8_t oe, bool dbuf,
       uint8_t width);
//private:

     uint8_t          nRows;
     RGB();
     uint8_t   _sclk, _latch, _oe, _a, _b, _c, _d,_e;

     volatile uint8_t row;
     volatile uint8_t *buffptr;
   };


#endif /* RGB_HPP_ */
