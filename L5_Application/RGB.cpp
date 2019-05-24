/*
 * RGB.cpp
 *
 *  Created on: Nov 14, 2018
 *      Author: neelpatel
 */

#include "RGB.hpp"
#include "gamma.h"
#include "stdio.h"
#include "string.h"
#include "LPC17xx.h"
#include "soft_timer.hpp"
#include "tasks.hpp"
#include "lpc_sys.h"
#include "lpc_rit.h"
#include "utilities.h"


void refreshISR(void);


#define R1  (1<<0)      // bit 2 = RED 1    P2.0
#define G1  (1<<1)      // bit 3 = GREEN 1  P2.1
#define B1  (1<<2)      // bit 4 = BLUE 1   P2.2
#define R2  (1<<3)      // bit 5 = RED 2    P2.3
#define G2  (1<<4)      // bit 6 = GREEN 2  P2.4
#define B2  (1<<5)      // bit 7 = BLUE 2   P2.5

RGB &displayUpdate = RGB::getRGBInstance();
volatile uint8_t colorbuffer[2048]={0};
RGB* RGB::rgbInstance=0;

RGB::RGB():GFX(64,64){

}

RGB& RGB::getRGBInstance()
{
    if(rgbInstance==0)
    {
        rgbInstance=new RGB();
    }
    return *rgbInstance;
}

void RGB::init(uint8_t rows, uint32_t a, uint32_t b, uint32_t c, uint32_t d , uint32_t e,uint8_t sclk, uint32_t latch, uint8_t oe, bool dbuf, uint8_t width2) {

    nRows = rows; // Number of multiplexed rows; actual height is 2X this
    // Save pin numbers for use by begin() method later.
    _a     = a;
    _b     = b;
    _c     = c;
    _d     = d;
    _e     = e;
    _sclk  = sclk;
    _latch = latch;
    _oe    = oe;

    row       = nRows   - 1;


    LPC_GPIO1->FIODIR |= (1<<_a);
    LPC_GPIO1->FIODIR |= (1<<_b);
    LPC_GPIO1->FIODIR |= (1<<_c);
    LPC_GPIO1->FIODIR |= (1<<_d);
    LPC_GPIO1->FIODIR |= (1<<_e);

    LPC_GPIO1->FIODIR |= (1<<_latch);

    LPC_GPIO2->FIODIR |= (1<<_sclk);
    LPC_GPIO2->FIODIR |= (1<<_oe);
    LPC_GPIO2->FIODIR |= R1;
    LPC_GPIO2->FIODIR |= G1;
    LPC_GPIO2->FIODIR |= B1;
    LPC_GPIO2->FIODIR |= R2;
    LPC_GPIO2->FIODIR |= G2;
    LPC_GPIO2->FIODIR |= B2;

    LPC_GPIO2->FIOSET = (1<<_oe);
    LPC_GPIO2->FIOCLR = (1<<_sclk);
    LPC_GPIO1->FIOCLR = (1<<_a);
    LPC_GPIO1->FIOCLR = (1<<_b);
    LPC_GPIO1->FIOCLR = (1<<_c);
    LPC_GPIO1->FIOCLR = (1<<_d);
    LPC_GPIO1->FIOCLR = (1<<_e);
    LPC_GPIO1->FIOCLR = (1<<_latch);
    LPC_GPIO2->FIOCLR = R1;
    LPC_GPIO2->FIOCLR = G1;
    LPC_GPIO2->FIOCLR = B1;
    LPC_GPIO2->FIOCLR = R2;
    LPC_GPIO2->FIOCLR = G2;
    LPC_GPIO2->FIOCLR = B2;

}


void RGB::begin(void) {

    LPC_SC->PCONP |= 1 << 1;    //Power up Timer 0
    LPC_SC->PCLKSEL0 |= 1 << 2; // Clock for timer = CCLK  GPIO interrupt
    LPC_TIM0->MR0 = 1 << 13;    // Give a value suitable for the LED blinking frequency based on the clock frequency

    LPC_TIM0->MCR |= 1 << 0;    // Interrupt on Match0 compare
    LPC_TIM0->MCR |= 1 << 1;    // Reset timer on Match 0.
    LPC_TIM0->TCR |= 1 << 1;    // Manually Reset Timer0 ( forced )
    LPC_TIM0->TCR &= ~(1 << 1); // stop resetting the timer.
    NVIC_EnableIRQ(TIMER0_IRQn); // Enable timer interrupt
    //isr_register(TIMER0_IRQn, TIMER0_IRQHandler);
    LPC_TIM0->TCR |= 1 << 0; // Start timer
}

void RGB::drawObstacle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour, bool isRectangle)
{
    if (isRectangle) {
        GFX::fillRect(x, y, w, h, colour);
    } else {
        GFX::drawCircle(x, y, w, colour);
    }
}

bool RGB::drawPixel(uint16_t x, uint16_t y, uint16_t c)
{
    uint8_t r, g, b;
    volatile uint8_t *coloradder, rgb_mask, r_mask, g_mask, b_mask;
    uint32_t base_addr;
    bool collision = false;

    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return true;

    /**
     * workaround for display offset bug
     * decrement rows for 32x64 segments with wrap-around
     */
    if (y == 0) {
        y = 31;
    } else if (y == 32) {
        y = 63;
    } else {
        y--;
    }

    r = (c >> 12) & 0xF;        // RRRRrggggggbbbbb
    g = (c >>  8) & 0xF;         // rrrrGGGGggbbbbb
    b = (c >>  1) & 0xF;         // rrrrgggggggBBBBb

    if (y < nRows) {
        base_addr = y * WIDTH  + x;
        rgb_mask = 0B00011100;
        r_mask = 0B00000100;
        g_mask = 0B00001000;
        b_mask = 0B00010000;
    } else {
        base_addr = (y - nRows) * WIDTH + x;
        rgb_mask = 0B11100000;
        r_mask = 0B00100000;
        g_mask = 0B01000000;
        b_mask = 0B10000000;
    }
    coloradder = &colorbuffer[base_addr];
    /**
     * collision detection included in generic drawPixel()
     * (NOTE adding extra assignment faster than separate function call, prevents duplicate code)
     */
    collision = (*coloradder & rgb_mask) != 0;
    *coloradder &= ~rgb_mask;            // Mask out R,G,B in one op

    if (r & 1) *coloradder |= r_mask; // Plane N R
    if (g & 1) *coloradder |= g_mask; // Plane N G
    if (b & 1) *coloradder |= b_mask; // Plane N B

    return collision;
}

void RGB::fillScreen(uint16_t c) {

    GFX::fillScreen(c);

}

void refreshISR(void)
{
    displayUpdate.updateDisplay();   // Call refresh func for active display
}

void RGB::updateDisplay(void) {
    uint8_t  i, *colorvalue;



    LPC_GPIO2->FIOCLR = (1<<_sclk);
    LPC_GPIO2->FIOSET = (1<<_oe);
    LPC_GPIO1->FIOCLR = (1<<_latch);


    if(++row >= nRows) {

        row     = 0;              // Yes, reset row counter, then...
        buffptr = &colorbuffer[0]; // Reset into front buffer
    }


    (row & 0x1) ? LPC_GPIO1->FIOSET = (1<<_a) : LPC_GPIO1->FIOCLR = (1<<_a);
    (row & 0x2) ? LPC_GPIO1->FIOSET = (1<<_b) : LPC_GPIO1->FIOCLR = (1<<_b);
    (row & 0x4) ? LPC_GPIO1->FIOSET = (1<<_c) : LPC_GPIO1->FIOCLR = (1<<_c);
    (row & 0x8) ? LPC_GPIO1->FIOSET = (1<<_d) : LPC_GPIO1->FIOCLR = (1<<_d);
    (row & 0x10) ? LPC_GPIO1->FIOSET = (1<<_e) : LPC_GPIO1->FIOCLR = (1<<_e);


    colorvalue = (uint8_t *)buffptr;


    for (i=0; i < WIDTH; i++) {

        (colorvalue[i] & 0x04) ? LPC_GPIO2->FIOSET = R1 : LPC_GPIO2->FIOCLR = R1;   //R1
        (colorvalue[i] & 0x08) ? LPC_GPIO2->FIOSET = G1 : LPC_GPIO2->FIOCLR = G1;   //G1
        (colorvalue[i] & 0x10) ? LPC_GPIO2->FIOSET = B1 : LPC_GPIO2->FIOCLR = B1;   //B1
        (colorvalue[i] & 0x20) ? LPC_GPIO2->FIOSET = R2 : LPC_GPIO2->FIOCLR = R2;    //R2
        (colorvalue[i] & 0x40) ? LPC_GPIO2->FIOSET = G2 : LPC_GPIO2->FIOCLR = G2;    //G2
        (colorvalue[i] & 0x80) ? LPC_GPIO2->FIOSET = B2 : LPC_GPIO2->FIOCLR = B2;    //B2
        LPC_GPIO2->FIOSET = (1<<_sclk);
        LPC_GPIO2->FIOCLR = (1<<_sclk);
    }

    buffptr += WIDTH;


    LPC_GPIO1->FIOSET = (1<<_latch);
    LPC_GPIO2->FIOCLR = (1<<_oe);


}



