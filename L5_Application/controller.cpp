#include "controller.hpp"
#include "acceleration_sensor.hpp"

Gpio button0 = Gpio(0);
QueueHandle_t toggle_led;

void toggle_led_task(void *p)
{
    const uint32_t port = 1, pin = 0; /* TODO 0, 1, 4, 8 */
    bool toggle;

    //vSemaphoreCreateBinary(toggle_led);
    toggle_led = xQueueCreate(1, sizeof(bool));
    if (!toggle_led) {
        u0_dbg_printf("ERROR: toggle led queue create\n");
    }
    Gpio led = Gpio(port, pin);
    led.setAsOutput();

    while(1) {
        if (xQueueReceive(toggle_led, &toggle, portMAX_DELAY)) {
            led.set(!led.getLevel());

            /* refresh LED settings */
            led.setAsOutput();
        }
    }
}

void init_button_detect_timer(void)
{
    LPC_SC->PCONP |= 1 << 1;    //Power up Timer 0
    LPC_SC->PCLKSEL0 |= 3 << 2; // Clock for timer = CCLK  GPIO interrupt
    LPC_TIM0->MR0 = 1 << 13;    // Give a value suitable for the LED blinking frequency based on the clock frequency

    LPC_TIM0->MCR |= 1 << 0;    // Interrupt on Match0 compare
    LPC_TIM0->MCR |= 1 << 1;    // Reset timer on Match 0.
    LPC_TIM0->TCR |= 1 << 1;    // Manually Reset Timer0 ( forced )
    LPC_TIM0->TCR &= ~(1 << 1); // stop resetting the timer.
    NVIC_EnableIRQ(TIMER0_IRQn); // Enable timer interrupt
    LPC_TIM0->TCR |= 1 << 0; // Start timer
}

void button_detect_isr(void)
{
    //UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR(); /* TODO */
    portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
    bool triggered = false, pressed = button0.getLevel();

    if ((LPC_TIM0->IR & 0x01) == 0x01) { /* if MR0 interrupt */
        LPC_TIM0->IR |= 1 << 0;
        if (xSemaphoreTake(button0.plock, (TickType_t)10)) {
            if ((button0.pressed && !pressed) || (!button0.pressed && pressed)) {
                triggered = true;
            }
            xSemaphoreGive(button0.plock);
        } else {
            u0_dbg_printf("ERROR: read button value failed\n");
        }
        if (triggered) {
            xQueueSendFromISR(button0.press_q, &pressed, &pxHigherPriorityTaskWoken);//TODO ozhu
        }

        //LPC_TIM0->IR |= 1 << 0;
    }
    //taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus); /* TODO */
}

void button_detect_task(void *p)
{
    bool pressed;

    button0.setAsInput();
    init_button_detect_timer();

    while (button0.press_q != 0) {
        if (xQueueReceive(button0.press_q, &pressed, portMAX_DELAY)) {
            if (xSemaphoreTake(button0.plock, (TickType_t)10)) {
                button0.pressed = pressed;
                xSemaphoreGive(button0.plock);
            } else {
                u0_dbg_printf("ERROR: read button task\n");
            }

            xQueueSend(toggle_led, &button0.pressed, (TickType_t)10);
            if (!button0.pressed) {
                send_message(ZZUM_CTRL, button0.__button);
            }

            /* refresh button settings; TODO handle freeze */
            button0.setAsInput();
            isr_register(TIMER0_IRQn, button_detect_isr);
            init_button_detect_timer();
        }
    }
    /* Only necessary if above loop has a condition */
    //xTaskDelete(NULL);
}

Gpio::Gpio(uint32_t button)
{
    __button = button;
    __port = 1;
    __pin = __button_pin_map[button];

    //button_gpio = Gpio(port, pin);
    set_gpio();

    plock = xSemaphoreCreateMutex();
    if (plock) {
        if (xSemaphoreTake(plock, portMAX_DELAY)) {
            pressed = false;
            xSemaphoreGive(plock);
        }
    } else {
        u0_dbg_printf("ERROR: mutex create\n");
        pressed = false;
    }

    press_q = xQueueCreate(1, sizeof(bool));
    if (!press_q) {
        u0_dbg_printf("ERROR: queue create\n");
    }
}

Gpio::Gpio(uint32_t port, uint32_t pin)
{
    __button = 0;
    __port = port;
    __pin = pin;

    set_gpio();

    /* unused */
    pressed = false;
    plock = 0;
    press_q = 0;

}

void Gpio::set_gpio()
{
    switch (__port) {
        case 0:
            __gpio = LPC_GPIO0;
            break;
        case 1:
            __gpio = LPC_GPIO1;
            break;
        case 2:
            __gpio = LPC_GPIO2;
            break;
        case 3:
            __gpio = LPC_GPIO3;
            break;
        case 4:
            __gpio = LPC_GPIO4;
            break;
        default:
            __gpio = LPC_GPIO1;
    }
}

void Gpio::setAsInput()
{
    __gpio->FIODIR &= ~(1 << __pin);
}

void Gpio::setAsOutput()
{
    __gpio->FIODIR |= (1 << __pin);
}

void Gpio::setDirection(bool output)
{
    if (output) {
        setAsOutput();
    } else {
        setAsInput();
    }
}

void Gpio::setHigh()
{
    __gpio->FIOSET = (1 << __pin);
}

void Gpio::setLow()
{
    __gpio->FIOCLR = (1 << __pin);
}

void Gpio::set(bool high)
{
    if (high) {
        setHigh();
    } else {
        setLow();
    }
}

bool Gpio::getLevel()
{
    if (__gpio->FIOPIN & (1 << __pin)) {
        return true;
    } else {
        return false;
    }
}

void send_message(uint32_t opcode, uint32_t d)
{
    uint32_t data;

#if 0
    mesh_packet_t pkt;
    wireless_form_pkt(&pkt, CONSOLE_NODE_ADDR, mesh_pkt_nack, 0, 2,
            &opcode, sizeof(opcode),
            &data, sizeof(data));
    wireless_send(CONSOLE_NODE_ADDR, mesh_pkt_nack, &accel_orientation, sizeof(accel_orientation), 0);
#endif

    if (opcode == ZZUM_CTRL) {
        data = ZZUM_CTRL;
    } else {
        data = d;
    }
    wireless_send(CONSOLE_NODE_ADDR, mesh_pkt_nack, &data, sizeof(data), 0);
}

void send_orientation(uint32_t orientation)
{
    send_message(ZZUM_MOVE, orientation);
}

void orient(void *p)
{
    int x = 0;
    uint32_t orientation;
    zzu_velocity velocity = no;

    while (1) {
        x = AS.getX();
        velocity = no;
        if (x >= 0) {
            if (x > 600) {
                velocity = uf;
            } else if (x > 400) {
                velocity = um;
            } else if (x > 200) {
                velocity = us;
            }
        } else {
            if (x < -600) {
                velocity = df;
            } else if (x < -400) {
                velocity = dm;
            } else if (x < -200) {
                velocity = ds;
            }
        }
        send_orientation(velocity);
        vTaskDelay(40);
    }
}

void wireless_transmit(void *p)
{
    uint32_t accel_orientation = 1;

    while (1) {
        wireless_send(CONSOLE_NODE_ADDR, mesh_pkt_nack, &accel_orientation, sizeof(accel_orientation), 0);
        vTaskDelay(80);
    }
}
