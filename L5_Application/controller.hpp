/*
 * controller.hpp
 *
 *  Created on: Dec 4, 2018
 */

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include "LPC17xx.h"
#include "FreeRTOS.h"
#include "printf_lib.h"
#include "semphr.h"

#include "io.hpp"

#include "messages.hpp"

class Gpio
{
private:
    uint32_t __port, __pin;
    LPC_GPIO_TypeDef *__gpio;
    uint32_t __button_pin_map[4] = { 9, 10, 14, 15 };
    void set_gpio();

public:
    Gpio(uint32_t button);
    Gpio(uint32_t port, uint32_t pin);
    ~Gpio() { ; };

    void setAsInput();
    void setAsOutput();
    void setDirection(bool output);
    void setHigh();
    void setLow();
    void set(bool high);
    bool getLevel();

    uint32_t __button;
    bool pressed;
    SemaphoreHandle_t plock;
    QueueHandle_t press_q;
    //Gpio button_gpio;
};

//extern QueueHandle_t button0_press_q;
///extern Button button0;

extern QueueHandle_t toggle_led;
void toggle_led_task(void *p);
void button_detect_isr(void);
void button_detect_task(void *p);

void orient(void *p);
void wireless_transmit(void *p);
void send_message(uint32_t opcode, uint32_t data);

#endif /* CONTROLLER_HPP_ */

