// rpi imports
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// lib imports
#include "../lib/pin-definitions.h"
#include "../lib/color.h"

#define MATRIX_ROWS 32
#define MATRIX_COLS 64

uint8_t bitplane_time = 1;      // in ms
Color framebuffer[MATRIX_ROWS][MATRIX_COLS];

void init_matrix_pins() {
    for (int pin = 5; pin < 20; pin++) {
        if (pin == 8) continue;

        gpio_init(pin);
        gpio_set_dir (pin, GPIO_OUT);
    }
}

void init_framebuffer() {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            framebuffer[row][col] = BLACK;
        }
    }
}

void init_matrix() {
    init_matrix_pins();
    init_framebuffer();

}

void init_color_test() {
    for (int col = 0; col < MATRIX_COLS; col++) {
        uint8_t r; uint8_t g; uint8_t b;

        if (col < 32) {
            r = 255 - col * 8;
            g = col * 8;
            b = 0;
        } else { // 32 <= col < 64
            r = 0;
            g = 255 - (col - 32) * 8;
            b = (col - 32) * 8;
        }

        framebuffer[0][col] = (Color) {r, g, b};
    }
}

void color_test() {
    init_color_test();

    // set row select to first row
    for (int row = A; row < D; row++) {
        gpio_put (row, 0);
    }

    for(;;) {
        for (int plane = 7; plane >= 0; plane--) {
            gpio_put(OE, 1);

            for (int col = 0; col < MATRIX_COLS; col++) {
                Color pixel = framebuffer[0][col];
                uint8_t r = pixel.r;
                uint8_t g = pixel.g;
                uint8_t b = pixel.b;

                gpio_put(R1, ((r >> plane) & 0x1));
                gpio_put(G1, ((g >> plane) & 0x1));
                gpio_put(B1, ((b >> plane) & 0x1));


                gpio_put(CLK, 1);
                sleep_us(1);
                gpio_put(CLK, 0);
            }

            gpio_put(LAT, 1);
            sleep_us(1);
            gpio_put(LAT, 0);
    
            gpio_put(OE, 0);
            sleep_us(1 << plane);
    
        }
    }

}

int main() {
    stdio_init_all();
    init_matrix();
    color_test();

    return 0;
}

/*  NOTES:

Row select; DCBA (meaning D = 8, C = 4, etc.)
    - This means 1011 = Row 11, 1001 = Row 9, etc 



*/