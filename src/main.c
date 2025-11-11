// rpi imports
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "joystick.h"
#include "lcd_display.h"

int main() {
    stdio_init_all();
    init_matrix();
    color_test();
  
    joystick_init();

    init_chardisp_pins();
    cd_init();

    const char *welcome[2] = { "   Bloons Game", " Press to Start" };
    print_message(welcome);
    sleep_ms(2000);

    // while (true) {
    //     int x = sample_x(); // -1, 0, or 1
    //     int y = sample_y(); // -1, 0, or 1
    //     bool pressed = joystick_select();

    //     printf("x=%d y=%d select=%d\r\n", x, y, pressed);

    //     sleep_ms(200);
    // }
}
