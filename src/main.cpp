// rpi imports
#include <stdio.h>
#include "pico/stdlib.h"

// lib imports
#include "tower.hh"
#include "matrix.hh"
#include "oled_display.hh"
#include "joystick.hh"

Tower towers[13] = {};
const char* welcome[2] = {"Round 1  $: 999 ", "         H: 100 " };
const char* purchase[2] = {"Ninja: 100      ", " Yes        No "};

int tower_idx = 0;
TowerType tower_type = dart;


void button_isr() {
    gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
    towers[tower_idx].type = tower_type;
    set_tower(towers[tower_idx++]);

    tower_type = tower_type < sniper ? (TowerType)(tower_type + 1) : dart;
}

void init_temp_button() {
    gpio_init(21);
    gpio_add_raw_irq_handler_masked(0b1 << 21, button_isr);
    
    gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE, 1);
    irq_set_enabled(IO_IRQ_BANK0, 1);

    gpio_init(22);
    gpio_set_dir(22, 1);
}

void init_monkeys() {
    towers[0].type = blank;
    towers[0].x_pos = 10;
    towers[0].y_pos = 2;

    towers[1].type = blank;
    towers[1].x_pos = 18;
    towers[1].y_pos = 7;

    towers[2].type = blank;
    towers[2].x_pos = 10;
    towers[2].y_pos = 12;

    towers[3].type = blank;
    towers[3].x_pos = 1;
    towers[3].y_pos = 18;

    towers[4].type = blank;
    towers[4].x_pos = 9;
    towers[4].y_pos = 23;

    towers[5].type = blank;
    towers[5].x_pos = 1;
    towers[5].y_pos = 28;

    towers[6].type = blank;
    towers[6].x_pos = 20;
    towers[6].y_pos = 34;

    towers[7].type = blank;
    towers[7].x_pos = 28;
    towers[7].y_pos = 38;

    towers[8].type = blank;
    towers[8].x_pos = 20;
    towers[8].y_pos = 42;

    towers[9].type = blank;
    towers[9].x_pos = 10;
    towers[9].y_pos = 49;

    towers[10].type = blank;
    towers[10].x_pos = 18;
    towers[10].y_pos = 54;

    towers[11].type = blank;
    towers[11].x_pos = 10;
    towers[11].y_pos = 59;

    towers[12].type = blank;
    towers[12].x_pos = 2;
    towers[12].y_pos = 59;
    
}

void init_base_game() {
    init_oled();
    print_message(purchase);
    
    init_js();

    init_monkeys();

    init_matrix();
    set_towers(towers);
    set_path();

    set_tree(2, 1);
    set_tree(10, 2);
    set_tree(3, 26);
    set_tree(15, 21);
    set_tree(24, 27);
    set_tree(38, 5);
    set_tree(45, 1);
    set_tree(53, 2);
    set_tree(51, 26);
    set_tree(58, 21);

    
}

int main() {
    stdio_init_all();

    init_temp_button();
    init_base_game();

    for (;;) {
        int x = sample_js_x();
        if (x == 1 && towers[0].type != blank) {
            towers[1].type = towers[0].type;
            towers[0].type = blank;
            set_tower(towers[0]);
            set_tower(towers[1]);

        }

        if (x == -1 && towers[1].type != blank) {
            gpio_put(22, 1);
            towers[0].type = towers[1].type;
            towers[1].type = blank;
            set_tower(towers[0]);
            set_tower(towers[1]);
        }

        render_frame();
    }

    return 0;

}

/*  NOTES:

Row select; DCBA (meaning D = 8, C = 4, etc.)
    - This means 1011 = Row 11, 1001 = Row 9, etc 
*/


/* 

// rpi imports
#include <stdio.h>
#include "hardware/gpio.h"

#include "hardware/adc.h"
#include "joystick.h"
#include "lcd_display.h"

int main() {
    stdio_init_all();
    init_matrix();
    color_test();
  
    joystick_init();
    oled_init();

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
    */