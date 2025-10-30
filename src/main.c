#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "joystick.h"

int main() {
    stdio_init_all();      // sets up UART for printf (115200 baud by default)
    joystick_init();       // sets up ADC on GPIO26/27 and pull-up on GPIO28

    while (1) {
        // read raw X
        adc_select_input(0); // X on ADC0 / GPIO40
        uint16_t raw_x = 4095 - adc_read();

        // read raw Y
        adc_select_input(1); // Y on ADC1 / GPIO41
        uint16_t raw_y = adc_read();

        bool S = joystick_select();

        printf("rawX=%u rawY=%u SEL=%d\r\n", raw_x, raw_y, S);
        sleep_ms(200);
    }
    
}