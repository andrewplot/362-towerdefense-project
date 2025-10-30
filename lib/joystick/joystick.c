#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define X_PIN 40   // X-axis to GPIO40 (channel 0)
#define Y_PIN 41   // Y-axis to GPIO41 (channel 1)
#define SW_PIN 42  // Switch to GPIO42

void joystick_init(void){
    adc_init();

    adc_gpio_init(X_PIN);
    adc_gpio_init(Y_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
}

bool joystick_left(void) {
    adc_select_input(0); // ADC0
    int value = adc_read(); // 0–4095
    return (value < 1000); // near 0V = left
}

bool joystick_right(void) {
    adc_select_input(0);
    int value = adc_read();
    return (value > 2000); // near 3.3V = right
}

bool joystick_up(void) {
    adc_select_input(1);
    uint16_t value = adc_read();
    return (value > 2000); // up = high voltage
}

bool joystick_down(void) {
    adc_select_input(1);
    uint16_t value = adc_read();
    return (value < 1000); // down = low voltage
}

bool joystick_select(void) {
    return (gpio_get(SW_PIN) == 0); // LOW when pressed
}