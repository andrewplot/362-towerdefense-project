#include "matrix.h"
#include <math.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "sprites.h"
#include "../pin-definitions.h"


#define MATRIX_ROWS 32
#define MATRIX_COLS 64

Color framebuffer[MATRIX_ROWS][MATRIX_COLS];
static uint8_t gamma_lut[256];

static inline void my_gpio_put(uint pin, bool val) {
    if (val) sio_hw->gpio_set = 1u << pin;
    else sio_hw->gpio_clr = 1u << pin;
}

static inline void set_row_pins(int row) {
    uint32_t pin_mask = (1u << A) | (1u << B) | (1u << C) | (1u << D);
    uint32_t pin_values = (((row >> 0) & 1) << A) |
                          (((row >> 1) & 1) << B) |
                          (((row >> 2) & 1) << C) |
                          (((row >> 3) & 1) << D);

    sio_hw->gpio_out = (sio_hw->gpio_out & ~pin_mask) | pin_values;
}

static inline void pulse_pin(int pin, int loops) {
    sio_hw->gpio_set = (1u << pin);
    for (volatile int i = 0; i < loops; ++i);
    sio_hw->gpio_clr = (1u << pin);
}

void init_matrix_pins() {
    gpio_init(25);
    gpio_set_dir(25, true);

    for (int pin = 5; pin < 20; pin++) {
        if (pin == 8) continue;

        gpio_init(pin);
        gpio_set_dir (pin, GPIO_OUT);
    }
}

void init_framebuffer(Color color) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            framebuffer[row][col] = color;
        }
    }
}

void init_gamma_lut() {
    float gamma = 3;

    for (int i = 0; i < 256; i++) {
        gamma_lut[i] = (uint8_t)(pow(i / 255.0, gamma) * 255.0);
    }
}

void init_matrix() {
    init_matrix_pins();
    init_framebuffer(GRASS);
    init_gamma_lut();
}

void reset_row_sel() {
    for (int row_sel = A; row_sel <= D; row_sel++) {
        my_gpio_put(row_sel, 0);
    }
}

void set_rgb_pins(int row, int col, int plane) {
    Color top = framebuffer[row][col];
    Color bottom = framebuffer[row+16][col];

    uint8_t top_r = gamma_lut[top.r];
    uint8_t top_g = gamma_lut[top.g];
    uint8_t top_b = gamma_lut[top.b];

    uint8_t bottom_r = gamma_lut[bottom.r];
    uint8_t bottom_g = gamma_lut[bottom.g];
    uint8_t bottom_b = gamma_lut[bottom.b];

    my_gpio_put(R1, ((top_r >> plane) & 0x1));
    my_gpio_put(G1, ((top_g >> plane) & 0x1));
    my_gpio_put(B1, ((top_b >> plane) & 0x1));

    my_gpio_put(R2, ((bottom_r >> plane) & 0x1));
    my_gpio_put(G2, ((bottom_g >> plane) & 0x1));
    my_gpio_put(B2, ((bottom_b >> plane) & 0x1));
}

void render_frame() {
    reset_row_sel();

    for (int plane = 5; plane >= 0; plane--) {
        for (int row = 0; row <  MATRIX_ROWS / 2; row++) {
            sio_hw->gpio_set = (1u << OE);
            sio_hw->gpio_set = (1u << 25);
            
            set_row_pins(row);
            
            for (int col = 0; col < MATRIX_COLS; col++) {
                set_rgb_pins(row, col, plane);
                pulse_pin(CLK, 3);
            }

            pulse_pin(LAT, 3);
            sio_hw->gpio_clr = (1u << OE);
            sio_hw->gpio_clr = (1u << 25);
            
            sleep_us(6 * (1 << plane));
        }
    }
}

void set_towers(Tower* towers) {
    for (int i = 0; i < 13; i++) {
        Tower tower = towers[i];
        Color* sprite = get_sprite(tower.type);

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                framebuffer[row + tower.x_pos][col + tower.y_pos] = sprite[row * 3 + col];
            }
        }
    }
}

void set_tower(Tower tower) {
    Color* sprite = get_sprite(tower.type);

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            framebuffer[row + tower.x_pos][col + tower.y_pos] = sprite[row * 3 + col];
            }
    }
}

void set_path() {
    for (int col = 0; col < 18; col++) {
        framebuffer[14][col] = PATH;
        framebuffer[15][col] = PATH;
        framebuffer[16][col] = PATH;

    }

    for (int row = 5; row < 17; row++) {
        framebuffer[row][16] = PATH;
        framebuffer[row][17] = PATH;
        framebuffer[row][18] = PATH;
    }
    
    for (int col = 16; col < 33; col++) {
        framebuffer[5][col] = PATH;
        framebuffer[6][col] = PATH;
        framebuffer[7][col] = PATH;

    }

    for (int row = 5; row < 27; row++) {
        framebuffer[row][30] = PATH;
        framebuffer[row][31] = PATH;
        framebuffer[row][32] = PATH;
    }

    for (int col = 33; col < 49; col++) {
        framebuffer[24][col] = PATH;
        framebuffer[25][col] = PATH;
        framebuffer[26][col] = PATH;
    }
    
    for (int row = 14; row < 27; row++) {
        framebuffer[row][46] = PATH;
        framebuffer[row][47] = PATH;
        framebuffer[row][48] = PATH;
    }

    for (int col = 46; col < 64; col++) {
        framebuffer[14][col] = PATH;
        framebuffer[15][col] = PATH;
        framebuffer[16][col] = PATH;
    }
}

void set_tree(int x, int y) {
    Color* sprite = get_sprite_tree();

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            framebuffer[row + y][col + x] = sprite[row * 3 + col];
        }
    }
}

void set_pixel(int x, int y, Color color) {
    framebuffer[y][x] = color;
}