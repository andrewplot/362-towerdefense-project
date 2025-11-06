#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "joystick.h"

int main() {
    stdio_init_all();
    joystick_init();

    while (true) {
        int x = sample_x(); // -1, 0, or 1
        int y = sample_y(); // -1, 0, or 1
        bool pressed = joystick_select();

        printf("x=%d y=%d select=%d\r\n", x, y, pressed);

        sleep_ms(200);
    }
}