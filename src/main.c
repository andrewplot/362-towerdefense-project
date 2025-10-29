#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "joystick.h"

int main() {
    // stdio_init_all();      // sets up UART for printf (115200 baud by default)
    // joystick_init();       // sets up ADC on GPIO26/27 and pull-up on GPIO28

    // while (1) {
    //     // read raw X
    //     adc_select_input(0);              // X on ADC0 / GPIO26
    //     uint16_t raw_x = adc_read();      // 0..4095

    //     // read raw Y
    //     adc_select_input(1);              // Y on ADC1 / GPIO27
    //     uint16_t raw_y = adc_read();      // 0..4095

    //     bool S = joystick_select();       // button

    //     printf("rawX=%u rawY=%u SEL=%d\r\n", raw_x, raw_y, S);
    //     sleep_ms(200);
    // }
    stdio_init_all();

    adc_init();
    adc_gpio_init(26);    // enable ADC on GPIO26

    while (1) {
        adc_select_input(0);        // ADC0 is GPIO26
        uint16_t reading = adc_read();  // 0..4095

        printf("ADC0=%u\r\n", reading);
        sleep_ms(200);
    }
}