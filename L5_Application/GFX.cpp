/*
 * GFX.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: neelpatel
 */

#include <stdint.h>
#include "GFX.hpp"
#include "glcdfont.c"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))


GFX::GFX(uint16_t w, uint16_t h):
  WIDTH(w), HEIGHT(h)
{
    printf("%d\n",w);
    printf("%d\n",h);

  _width    = WIDTH;
  _height   = HEIGHT;
  cursor_y  = cursor_x    = 0;
  textsize  = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap      = true;
}

void GFX::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t steep =  abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  uint16_t err = dx / 2;
  uint16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/* TODO duplicate code */
bool GFX::drawLineCollision(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t *x_c, uint16_t *y_c)
{

    bool collision = false, collision_tmp = false;
    uint16_t steep =  abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    uint16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    uint16_t err = dx / 2;
    uint16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            //collision = drawPixel(y0, x0, color) || collision;
            if ((collision_tmp = drawPixel(y0, x0, color))) {
                *x_c = y0;
                *y_c = x0;
            }
        } else {
            //collision = drawPixel(x0, y0, color) || collision;
            if ((collision_tmp = drawPixel(x0, y0, color))) {
                *x_c = x0;
                *y_c = y0;
            }
        }
        collision = collision_tmp || collision;
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
    return collision;
}

void GFX::drawFastVLine(uint16_t x, uint16_t y,uint16_t h, uint16_t color) {
  drawLine(x, y, x, y+h-1, color);
}

void GFX::drawFastHLine(uint16_t x, uint16_t y,uint16_t w, uint16_t color) {
    drawLine(x, y, x+w-1, y, color);
}

void GFX::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t color) {

  for (uint16_t i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

void GFX::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}



uint8_t GFX::write(uint8_t c) {

  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x  = 0;
  } else if (c == '\r') {
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    if (wrap && (cursor_x > (_width - textsize*6))) {
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }

  return 1;

}

void GFX::drawCircle(int16_t x0, int16_t y0, int16_t r,
        uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    //startWrite();
    drawPixel(x0  , y0+r, color);
    drawPixel(x0  , y0-r, color);
    drawPixel(x0+r, y0  , color);
    drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
    //endWrite();
}
void GFX::drawChar(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5)
      line = 0x0;
    else
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        }
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

void GFX::setCursor(uint16_t x, uint16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void GFX::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void GFX::setTextColor(uint16_t c) {

  textcolor = textbgcolor = c;
}

void GFX::setTextColor(uint16_t c, uint16_t b) {
  textcolor   = c;
  textbgcolor = b;
}

void GFX::setTextWrap(bool w) {
  wrap = w;
}

/*
void GFX::setRotation(uint8_t x) {
  rotation = (x & 3);
  switch(rotation) {
   case 0:
   case 2:
    _width  = WIDTH;
    _height = HEIGHT;
    break;
   case 1:
   case 3:
    _width  = HEIGHT;
    _height = WIDTH;
    break;
  }
}
*/

uint16_t GFX::width(void) const {
  return _width;
}

uint16_t GFX::height(void) const {
  return _height;
}


uint8_t GFX::write(const uint8_t *buffer, uint8_t size)
{
    uint8_t n = 0;
  while (size--) {
    n += write(*buffer++);
  }
  return n;
}

uint8_t GFX::print(const char str[])
{
  return write(str);
}

uint8_t GFX::print(char c)
{
  return write(c);
}

uint8_t GFX::print(unsigned char b, int base)
{
  return print((unsigned long) b, base);
}

uint8_t GFX::print(int n, int base)
{
  return print((long) n, base);
}

uint8_t GFX::print(unsigned int n, int base)
{
  return print((unsigned long) n, base);
}

uint8_t GFX::print(long n, int base)
{
  if (base == 0) {
    return write(n);
  } else if (base == 10) {
    if (n < 0) {
      int t = print('-');
      n = -n;
      return printNumber(n, 10) + t;
    }
    return printNumber(n, 10);
  } else {
    return printNumber(n, base);
  }
}

uint8_t GFX::print(unsigned long n, int base)
{
  if (base == 0) return write(n);
  else return printNumber(n, base);
}

uint8_t GFX::print(double n, int digits)
{
  return printFloat(n, digits);
}





uint8_t GFX::println(void)
{
  size_t n = print('\r');
  n += print('\n');
  return n;
}



uint8_t GFX::println(const char c[])
{
  size_t n = print(c);
  n += println();
  return n;
}

uint8_t GFX::println(char c)
{
  size_t n = print(c);
  n += println();
  return n;
}

uint8_t GFX::println(unsigned char b, int numBase)
{
  size_t n = print(b, numBase);
  n += println();
  return n;
}

uint8_t GFX::println(int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

uint8_t GFX::println(unsigned int num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

uint8_t GFX::println(long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

uint8_t GFX::println(unsigned long num, int base)
{
  size_t n = print(num, base);
  n += println();
  return n;
}

uint8_t GFX::println(double num, int digits)
{
  size_t n = print(num, digits);
  n += println();
  return n;
}




uint8_t GFX::printNumber(unsigned long n, uint8_t base) {
  char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  if (base < 2) base = 10;

  do {
    unsigned long m = n;
    n /= base;
    char c = m - base * n;
    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  return write(str);
}

uint8_t GFX::printFloat(double number, uint8_t digits)
{
    uint8_t n = 0;

  if (isnan(number)) return print("nan");
  if (isinf(number)) return print("inf");
  if (number > 4294967040.0) return print ("ovf");  // constant determined empirically
  if (number <-4294967040.0) return print ("ovf");  // constant determined empirically

  if (number < 0.0)
  {
     n += print('-');
     number = -number;
  }

  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;

  number += rounding;

  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  n += print(int_part);

  if (digits > 0) {
    n += print(".");
  }

  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    n += print(toPrint);
    remainder -= toPrint;
  }

  return n;
}



