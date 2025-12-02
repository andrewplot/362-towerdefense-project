// src/main.cpp - Main game loop for RP2350

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "../game_types.h"

// ---------------------------------------------------------------------------
// Forward declarations for matrix functions
// (avoid including matrix.hh to dodge the Tower type collision)
// ---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// These signatures match lib/led_matrix/matrix.hh / matrix.cpp
void init_matrix();
void swap_frames();
void render_frame();
void set_pixel(int x, int y, Color color);

#ifdef __cplusplus
}
#endif

// Joystick header – use the real one
#include "../lib/joystick/joystick.hh"

// ---------------------------------------------------------------------------
// Global framebuffer defined here (declared extern in game_types.h)
// ---------------------------------------------------------------------------

Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

// ---------------------------------------------------------------------------
// Game + timing state
// ---------------------------------------------------------------------------

GameState game;
uint32_t  last_time_ms = 0;

// Current tower selection for testing
TowerType current_tower_selection = TOWER_MACHINE_GUN;
bool      button_pressed_last_frame = false;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {
    stdio_init_all();

    // Initialize peripherals
    init_matrix();     // sets up pins, framebuffers, gamma, fills with GRASS
    init_joystick();   // from joystick.hh

    // Initialize game state
    game_init(&game);

    // Spawn a test enemy so you see something moving immediately
    game_spawn_enemy(&game, ENEMY_SCOUT);

    printf("Tower Defense Game Started!\n");
    printf("Controls:\n");
    printf("  Joystick: Move cursor (future)\n");
    printf("  Button: Place tower on first slot (test)\n");
}

// ---------------------------------------------------------------------------
// Input handling (joystick)
// ---------------------------------------------------------------------------

void handle_input() {
    // Read joystick values from your joystick library
    int  joy_x = sample_js_x();
    int  joy_y = sample_js_y();
    bool button = sample_js_select();

    (void)joy_x;
    (void)joy_y;
    // TODO: use joy_x / joy_y to move a cursor over tower_slots

    // For now: pressing the button once tries to place a tower in slot 0
    if (button && !button_pressed_last_frame) {
        if (game.tower_slot_count > 0) {
            int16_t tx = game.tower_slots[0].x;
            int16_t ty = game.tower_slots[0].y;

            if (game_place_tower(&game, current_tower_selection, tx, ty)) {
                printf("Placed tower! Money: %d\n", game.money);
            } else {
                printf("Cannot place tower (not enough money or slot occupied)\n");
            }
        }
    }

    button_pressed_last_frame = button;
}

// ---------------------------------------------------------------------------
// Game update
// ---------------------------------------------------------------------------

void update_game() {
    uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
    float    dt = (current_time_ms - last_time_ms) / 1000.0f;
    last_time_ms = current_time_ms;

    // Clamp in case of long pauses
    if (dt > 0.1f) dt = 0.1f;

    game_update(&game, dt);

    // Simple periodic spawner for testing
    static float spawn_timer = 0.0f;
    spawn_timer += dt;
    if (spawn_timer > 3.0f) {
        if (game.enemy_count < MAX_ENEMIES) {
            game_spawn_enemy(&game, ENEMY_SCOUT);
            printf("Spawned enemy. Total: %d\n", game.enemy_count);
        }
        spawn_timer = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Render into game framebuffer
// ---------------------------------------------------------------------------

void render_game() {
    // Clear background to grass
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            framebuffer[y][x] = GRASS;  // GRASS is defined in color.hh
        }
    }

    // Let the game draw enemies, towers, bullets, etc. into framebuffer
    game_draw(&game);
}

// ---------------------------------------------------------------------------
// Push framebuffer to LED matrix and show it
// ---------------------------------------------------------------------------

void render() {
    // Copy our logical framebuffer into the matrix driver's backbuffer
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            set_pixel(x, y, framebuffer[y][x]);
        }
    }

    // Double-buffering: swap, then render the now-current display buffer
    swap_frames();
    render_frame();
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main() {
    setup();

    last_time_ms = to_ms_since_boot(get_absolute_time());

    // Main game loop: input -> update -> draw -> display
    while (true) {
        handle_input();
        update_game();
        render_game();  // fills framebuffer
        render();       // sends framebuffer to the actual LED panel
        sleep_ms(33);   // ~30 FPS
    }

    return 0;
}
