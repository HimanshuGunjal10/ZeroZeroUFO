/*
 * console.hpp
 *
 *  Created on: Dec 1, 2018
 */

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include "LPC17xx.h"
#include "FreeRTOS.h"
#if 1
#include "task.h"
#endif
#include "printf_lib.h"
#include <time.h>
#include "utilities.h"

#include "RGB.hpp"
#include "GFX.hpp"

#include "messages.hpp"
#include "semphr.h"

/* Colors */
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x0202
#define CYAN    0x01FF
#define VIOLET  0xFFE0
#define BLUE    0x85DD
#define PINK    0xFF00
#define BLACK   0x0000
#define GOLD    0xF81F

/* Game states */
const uint32_t ZZUS_INIT = 0;
const uint32_t ZZUS_RESET = 1;
const uint32_t ZZUS_PLAY = 2;

extern RGB rgb;
extern SemaphoreHandle_t update_display_semphr;
extern QueueHandle_t orientation_q;
extern QueueHandle_t control_q;
extern QueueHandle_t score_q;

void timer0_ISR(void);
void update_display_task(void *p);
void receive_message(void *p);
void start_screen(void *p);
void boom_score(void *p);
void gameplay(void *p);

extern TaskHandle_t gameplay_h;
extern TaskHandle_t start_h;

const uint32_t MAX_OBSTACLES = 5;
const uint32_t MAX_SPEED = 3;

void summon_evil(uint32_t *y_evil, bool *summoned);
void ufo_evil_reset(uint32_t *y_evil, bool *summoned);

/* TODO better class hierarchy between different types of obstacles, coins, lifes */
class Obstacle
{
private:
    uint16_t obstacle_colours[4] = { RED, GREEN, CYAN, BLUE };
    enum Shape {
        rectangle,
        circle
    };
    enum Speed {
        slow,
        medium,
        fast
    };

    uint16_t x, y, w, h, c;
    uint32_t speed;
    Shape shape;
    int fire_angle;
    int fire_count;

    void setShape();
    void setColour();
    void setSpeed(uint32_t score);

public:
    Obstacle();
    ~Obstacle() { ; };

    bool fired;
    void init();
    void init(uint32_t score);
    void initLaser();
    void fireLaser(uint32_t y_u);
    void shoot();
    void draw();
    void erase();
    void shift();
    bool done();
    bool isSlow();
    bool isMed();
    bool isFast();
};

class Coin
{
private:
    uint16_t x, y;

public:
    Coin();
    ~Coin() { ; };

    void init();
    void draw();
    void erase();
    void shift();
    bool done();
    bool collide(uint16_t x_collide, uint16_t y_collide);
};

#endif /* CONSOLE_HPP_ */
