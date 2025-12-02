#include <stdio.h>
#include <stdbool.h>
#include <string>
#include "pico/stdlib.h"
#include "pico/multicore.h"

// lib imports
#include "rfid.hh"
#include "tower.hh"
#include "matrix.hh"
#include "oled_display.hh"
#include "joystick.hh"
#include "buzzer_pwm.hh"

TowerType scanned_tower = blank;
char *towers[] = {"Dart Monkey", "Ninja Monkey", "Bomb Tower", "Sniper Monkey"};

JoystickDirection last_x = center;
JoystickDirection last_y = center;
char *directions[] = {"Left", "Right", "Up", "Down", "Center" };
bool last_select = false;

void sample_peripherals() {
    if (rfid_flag) {
        rfid_flag = false;
        scanned_tower = sample_rfid();
        printf("Scanned Tower: %d\n", scanned_tower);
    }

    if (joystick_flag) {
        joystick_flag = false;
        JoystickDirection x = sample_js_x();
        JoystickDirection y = sample_js_y();
        bool select = sample_js_select();

        if (x != last_x) {
            printf("Joystick X: %s\n", directions[x]);
            last_x = x;
        } 
        if (y != last_y) {
            printf("Joystick Y: %s\n", directions[y]);
            last_y = y;
        }
        if (select != last_select) {
            printf("Joystick Sel: %s\n", select ? "true" : "false");
            last_select = select;
        }
    }
}

void init_peripherals() {
    init_rfid();
    init_joystick();
    init_oled();
    buzzer_pwm_init();
}

void render_matrix() {
    for (;;) {
        render_frame();
    }
}

int main() {
    stdio_init_all();
    
    // Give time for USB serial to connect
    sleep_ms(3000);
    init_peripherals();
    
    oled_print("Hello \1", "I have mucho \2");
    start_sound();

    for (;;) {
        sample_peripherals();

        // render_game
        // call multicore_fifo_push_blocking(1);
    }
    
}

// void init_monkeys() {
//     towers[0].type = blank;
//     towers[0].x_pos = 10;
//     towers[0].y_pos = 2;

//     towers[1].type = blank;
//     towers[1].x_pos = 18;
//     towers[1].y_pos = 7;

//     towers[2].type = blank;
//     towers[2].x_pos = 10;
//     towers[2].y_pos = 12;

//     towers[3].type = blank;
//     towers[3].x_pos = 1;
//     towers[3].y_pos = 18;

//     towers[4].type = blank;
//     towers[4].x_pos = 9;
//     towers[4].y_pos = 23;

//     towers[5].type = blank;
//     towers[5].x_pos = 1;
//     towers[5].y_pos = 28;

//     towers[6].type = blank;
//     towers[6].x_pos = 20;
//     towers[6].y_pos = 34;

//     towers[7].type = blank;
//     towers[7].x_pos = 28;
//     towers[7].y_pos = 38;

//     towers[8].type = blank;
//     towers[8].x_pos = 20;
//     towers[8].y_pos = 42;

//     towers[9].type = blank;
//     towers[9].x_pos = 10;
//     towers[9].y_pos = 49;

//     towers[10].type = blank;
//     towers[10].x_pos = 18;
//     towers[10].y_pos = 54;

//     towers[11].type = blank;
//     towers[11].x_pos = 10;
//     towers[11].y_pos = 59;

//     towers[12].type = blank;
//     towers[12].x_pos = 2;
//     towers[12].y_pos = 59;
    
// }

// void init_base_game() {
//     init_oled();
//     print_message(purchase);
    
//     init_js();

//     init_monkeys();

//     init_matrix();
//     set_towers(towers);
//     set_path();

//     set_tree(2, 1);
//     set_tree(10, 2);
//     set_tree(3, 26);
//     set_tree(15, 21);
//     set_tree(24, 27);
//     set_tree(38, 5);
//     set_tree(45, 1);
//     set_tree(53, 2);
//     set_tree(51, 26);
//     set_tree(58, 21);
// }

// }

// void game_test() {

//     init_temp_button();
//     init_base_game();

//     for (;;) {
//         int x = sample_js_x();
//         if (x == 1 && towers[0].type != blank) {
//             towers[1].type = towers[0].type;
//             towers[0].type = blank;
//             set_tower(towers[0]);
//             set_tower(towers[1]);

//         }

//         if (x == -1 && towers[1].type != blank) {
//             gpio_put(22, 1);
//             towers[0].type = towers[1].type;
//             towers[1].type = blank;
//             set_tower(towers[0]);
//             set_tower(towers[1]);
//         }

//         render_frame();
//     }

// }