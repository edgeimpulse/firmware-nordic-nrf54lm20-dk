/*
 * Copyright (c) 2026 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ei_at_handlers.h"
#include "ei_device_nordic.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "inference/ei_run_impulse.h"
#include "sensors/ei_inertial.h"
#include "sensors/ei_accelerometer.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <nrfx_clock.h>
#include <zephyr/kernel.h>
//#include <zephyr/bluetooth/bluetooth.h>

#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
    EiDeviceNRF *dev = static_cast<EiDeviceNRF*>(EiDeviceInfo::get_device());
    ATServer *at;
    int err = 0;

    /* output of printf is output immediately without buffering */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Initialize board uart */
    if(uart_init() != 0) {
        LOG_ERR("Init uart on board error occured\r\n");
    }

    at = ei_at_init();

    /* Setup the inertial sensor */
    if(ei_inertial_init() == false) {
        LOG_ERR("[ERR] Can't init inertial sensor");
        if(gpio_is_ready_dt(&led0) && gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) == 0) {
            gpio_pin_set_dt(&led0, 1);
        }
    }

    /* Setup the accelerometer sensor */
    if(ei_accelerometer_init() == false) {
        LOG_ERR("[ERR] Can't init accelerometer sensor");
        if(gpio_is_ready_dt(&led0) && gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) == 0) {
            gpio_pin_set_dt(&led0, 1);
        }
    }

    ei_printf("Hello from Edge Impulse\r\n"
              "Compiled on %s %s\r\n", __DATE__, __TIME__);

    at->print_prompt();
    LOG_INF("Entering infinite loop\n");
    dev->init_device_id();
    while(1) {
        char data = uart_getchar();

        while(data != 0xFF) {
            if(is_inference_running() && data == 'b') {
                ei_stop_impulse();
                at->print_prompt();
                continue;
            }
            dev->set_serial_channel(UART);
            at->handle(data);
            data = uart_getchar();
        }
        ei_sleep(1);
    }
}
