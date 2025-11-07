// rpi imports
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// lib imports
#include "matrix.h"

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
            framebuffer[row][col] = GRASS;
        }
    }
}

void init_matrix() {
    init_matrix_pins();
    init_framebuffer();
}

void render() {
    reset_row_sel();

    int row = 0;
    for(;;) {
        for (int plane = 7; plane >= 0; plane--) {
            for (int row = 0; row <  MATRIX_ROWS / 2; row++) {
                sio_hw->gpio_set = (1u << OE);
                
                set_row_pins(row);
                
                for (int col = 0; col < MATRIX_COLS; col++) {
                    set_rgb_pins(row, col, plane);
                    pulse_pin(CLK, 3);
                }

                pulse_pin(LAT, 3);
                sio_hw->gpio_clr = (1u << OE);
                sleep_us(2 * (1 << plane));
            }
        
        }
    }

}


void reset_row_sel() {
    for (int row_sel = A; row_sel <= D; row_sel++) {
        my_gpio_put(row_sel, 0);
    }
}

inline void pulse_pin(int pin, int loops) {
    sio_hw->gpio_set = (1u << pin);
    for (volatile int i = 0; i < loops; ++i);
    sio_hw->gpio_clr = (1u << pin);
}

void set_rgb_pins(int row, int col, int plane) {
    Color top = framebuffer[row][col];

    my_gpio_put(R1, ((top.r >> plane) & 0x1));
    my_gpio_put(G1, ((top.g >> plane) & 0x1));
    my_gpio_put(B1, ((top.b >> plane) & 0x1));

    Color bottom = framebuffer[row+16][col];
    my_gpio_put(R2, ((bottom.r >> plane) & 0x1));
    my_gpio_put(G2, ((bottom.g >> plane) & 0x1));
    my_gpio_put(B2, ((bottom.b >> plane) & 0x1));
}

inline void my_gpio_put(uint pin, bool val) {
    if (val) sio_hw->gpio_set = 1u << pin;
    else sio_hw->gpio_clr = 1u << pin;
}

inline void set_row_pins(int row) {
    uint32_t pin_mask = (1u << A) | (1u << B) | (1u << C) | (1u << D);
    uint32_t pin_values = (((row >> 0) & 1) << A) |
                          (((row >> 1) & 1) << B) |
                          (((row >> 2) & 1) << C) |
                          (((row >> 3) & 1) << D);

    sio_hw->gpio_out = (sio_hw->gpio_out & ~pin_mask) | pin_values;
}
/*  NOTES:

    Row select; DCBA (meaning D = 8, C = 4, etc.)
        - This means 1011 = Row 11, 1001 = Row 9, etc 

*/