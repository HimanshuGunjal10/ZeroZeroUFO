/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
 /**
 * @file
 * @brief This is the application entry point.
 *          FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 *          @see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include "tasks.hpp"
#include "examples/examples.hpp"

#ifdef ZZU_CONTROLLER
#include "controller.hpp"
#endif /* ZZU_CONTROLLER */

#ifdef ZZU_CONSOLE
#include "console.hpp"
#endif /* ZZU_CONSOLE */

#if 0
#include "uart0_min.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "io.hpp"
//#include "spi_sem.h"
#include "event_groups.h"
#include "mpu_wrappers.h"
#include "task.h"
#include "command_handler.hpp"
#include "string.h"
#include "storage.hpp"
#include "i2c2.hpp"
#endif

int main(void)
{
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));

#if 0
    const bool run_1Khz = false;
    scheduler_add_task(new periodicSchedulerTask(run_1Khz));

    Uart3 &u3 = Uart3::getInstance();
    u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
    scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
#endif

#ifdef ZZU_CONTROLLER
    xTaskCreate(orient, "orient", 512, NULL, PRIORITY_MEDIUM, NULL);
    //xTaskCreate(wireless_transmit, "wireless_transmit", 256, NULL, PRIORITY_LOW, NULL);

    isr_register(TIMER0_IRQn, button_detect_isr);
    xTaskCreate(toggle_led_task, "led", 256, 0, PRIORITY_HIGH + 1, NULL);
    xTaskCreate(button_detect_task, "button", 256, 0, PRIORITY_HIGH, NULL);
#endif /* ZZU_CONTROLLER */

#ifdef ZZU_CONSOLE
    update_display_semphr = xSemaphoreCreateBinary();
    orientation_q = xQueueCreate(1, sizeof(uint32_t));
    control_q = xQueueCreate(1, sizeof(uint32_t));
    score_q = xQueueCreate(1, sizeof(uint32_t));

    isr_register(TIMER0_IRQn, timer0_ISR);
    rgb.init(32, 19, 20, 22, 23, 28, 6, 29, 7, true, 64);
    rgb.begin();

    //xTaskCreate(update_display_task, "update display", 256, NULL, PRIORITY_HIGH, NULL);

    srand(time(0));
    xTaskCreate(boom_score, "BOOM", 256, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(start_screen, "start", 256, NULL, PRIORITY_HIGH, &start_h);
    //xTaskCreate(RGB_UFO, "UFO", 256, NULL, 1, NULL);
    //xTaskCreate(gameplay, "Game_Screen", 256, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(receive_message, "recv", 256, NULL, PRIORITY_HIGH, NULL);
#endif /* ZZU_CONSOLE */

    scheduler_start(); ///< This shouldn't return
    return -1;
}
