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
    init_matrix();
}

void render_matrix() {
    for (;;) {
        render_frame();
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking();
            swap_frames();
        }
    }
}

int main() {
    stdio_init_all();
    
    // Give time for USB serial to connect
    sleep_ms(3000);
    init_peripherals();
    
    multicore_launch_core1(render_matrix);
    oled_print("Hello \1", "I have mucho \2");
    start_sound();

    for (;;) {
        sample_peripherals();
        
        //render_game by calling set_pixel(x, y, Color)
        Tower t1;
        t1.type = ninja;
        t1.x_pos = 10;
        t1.y_pos = 10;

        set_tower(t1);

        multicore_fifo_push_blocking(1);
        sleep_ms(250);
        // call multicore_fifo_push_blocking(1); to swap matrix frames

    }
}  
