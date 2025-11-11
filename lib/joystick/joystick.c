#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "../pin-definitions.h"


int sample_x(void){
    adc_select_input(0);
    uint16_t value = adc_read();

    if (value > 3200) return 1;
    else if (value < 900) return -1;
    else return 0;
}

int sample_y(void){
    adc_select_input(1);
    uint16_t value = adc_read();

    if (value > 3200) return 1;
    else if (value < 900) return -1;
    else return 0;
}

bool joystick_select(void) {
    return (gpio_get(JOYSTICK_SW) == 0); // LOW when pressed
}

void joystick_init(void){
    adc_init();

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_SW);
    gpio_set_dir(JOYSTICK_SW, GPIO_IN);
    gpio_pull_up(JOYSTICK_SW);
}

// bool joystick_left(void) {
//     adc_select_input(0); // ADC0
//     int value = adc_read(); // 0–4095
//     return (value < 1000); // near 0V = left
// }

// bool joystick_right(void) {
//     adc_select_input(0);
//     int value = adc_read();
//     return (value > 2000); // near 3.3V = right
// }

// bool joystick_up(void) {
//     adc_select_input(1);
//     uint16_t value = adc_read();
//     return (value > 2000); // up = high voltage
// }

// bool joystick_down(void) {
//     adc_select_input(1);
//     uint16_t value = adc_read();
//     return (value < 1000); // down = low voltage
// }