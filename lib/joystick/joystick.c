#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "../pin-definitions.h"

void init_js(void){
    adc_init();

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_SW);
    gpio_set_dir(JOYSTICK_SW, GPIO_IN);
    gpio_pull_up(JOYSTICK_SW);
}

int sample_js_x(void){
    adc_select_input(0);
    uint16_t value = adc_read();

    int deadzone = (ADC_MAX / 2) * DEADZONE_PERCENT / 100;
    if (value > CENTER + deadzone) return 1;   // right
    else if (value < CENTER - deadzone) return -1;  // left
    else return 0;
}

int sample_js_y(void){
    adc_select_input(1);
    uint16_t value = adc_read();

    int deadzone = (ADC_MAX / 2) * DEADZONE_PERCENT / 100;
    if (value > CENTER + deadzone) return 1;   // up
    else if (value < CENTER - deadzone) return -1;  // down
    else return 0;
}

bool sample_js_select(void) {
    return (gpio_get(JOYSTICK_SW) == 0); // LOW when pressed
}

