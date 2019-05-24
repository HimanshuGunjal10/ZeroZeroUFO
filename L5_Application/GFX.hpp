/*
 * GFX.hpp
 *
 *  Created on: Nov 14, 2018
 *      Author: neelpatel
 */

#ifndef GFX_HPP_
#define GFX_HPP_


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define DEC 10

#define swap(a, b) { uint16_t t = a; a = b; b = t; }

class GFX  {

 public:

  GFX(uint16_t w, uint16_t h);
 // virtual ~GFX();

  virtual bool drawPixel(uint16_t x, uint16_t y, uint16_t color) = 0;
  virtual void
    drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color),
    drawFastVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color),
    drawFastHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color),
    fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color),
    fillScreen(uint16_t color);
  bool drawLineCollision(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t *x_c, uint16_t *y_c);

  void

    drawChar(uint16_t x, uint16_t y, unsigned char c, uint16_t color,

      uint16_t bg, uint8_t size),
      drawCircle(int16_t x0, int16_t y0, int16_t r,
              uint16_t color),
    setCursor(uint16_t x, uint16_t y),
    setTextColor(uint16_t c),
    setTextColor(uint16_t c, uint16_t bg),
    setTextSize(uint8_t s),
    setTextWrap(bool w);
   // setRotation(uint8_t r);


  uint16_t height(void) const;
  uint16_t width(void) const;

 // uint8_t getRotation(void) const;

 protected:
  const uint16_t
    WIDTH, HEIGHT;
  uint16_t
    _width, _height,
    cursor_x, cursor_y;
  uint16_t
    textcolor, textbgcolor;
  uint8_t
    textsize;
  bool
    wrap;


 private:
  uint8_t printNumber(unsigned long, uint8_t);
  uint8_t printFloat(double, uint8_t);
public:
  uint8_t write(uint8_t);
  uint8_t write(const char *str) {
        if (str == NULL) return 0;
        return write((const uint8_t *)str, strlen(str));
      }
  uint8_t write(const uint8_t *buffer, uint8_t size);
  uint8_t write(const char *buffer, uint8_t size) {
        return write((const uint8_t *)buffer, size);
      }

    uint8_t print(const char[]);
     uint8_t print(char);
     uint8_t print(unsigned char, int = DEC);
     uint8_t print(int, int = DEC);
     uint8_t print(unsigned int, int = DEC);
     uint8_t print(long, int = DEC);
     uint8_t print(unsigned long, int = DEC);
     uint8_t print(double, int = 2);

     uint8_t println(const char[]);
     uint8_t println(char);
     uint8_t println(unsigned char, int = DEC);
     uint8_t println(int, int = DEC);
     uint8_t println(unsigned int, int = DEC);
     uint8_t println(long, int = DEC);
     uint8_t println(unsigned long, int = DEC);
     uint8_t println(double, int = 2);

     uint8_t println(void);
};


#endif /* GFX_HPP_ */
