// src/main.cpp - Main game loop for RP2350
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Project includes
#include "../lib/matrix/matrix.h"
#include "../lib/joystick/joystick.h"
#include "game_types.h"

// Game state
GameState game;
uint32_t last_time_ms = 0;

// Input state
TowerType current_tower_selection = TOWER_MACHINE_GUN;
bool button_pressed_last_frame = false;

void setup() {
    stdio_init_all();

    // Initialize peripherals
    init_matrix();
    joystick_init();

    // Initialize game
    game_init(&game);

    // Spawn some test enemies
    game_spawn_enemy(&game, ENEMY_SCOUT);

    printf("Tower Defense Game Started!\n");
    printf("Controls:\n");
    printf("  Joystick: Move cursor\n");
    printf("  Button: Place tower\n");
}

void handle_input() {
    int joy_x = sample_x();
    int joy_y = sample_y();
    bool button = joystick_select();

    // TODO: use joystick to move a cursor over tower slots
    // For now: button places selected tower at first slot for testing
    if (button && !button_pressed_last_frame) {
        if (game_place_tower(&game,
                             current_tower_selection,
                             game.tower_slots[0].x,
                             game.tower_slots[0].y)) {
            printf("Placed tower! Money: %d\n", game.money);
        } else {
            printf("Cannot place tower (money/slot)\n");
        }
    }

    button_pressed_last_frame = button;
}

void update_game() {
    uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
    float dt = (current_time_ms - last_time_ms) / 1000.0f;
    last_time_ms = current_time_ms;

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

void render_game() {
    // Clear background to grass
    for (int y = 0; y < MATRIX_ROWS; y++) {
        for (int x = 0; x < MATRIX_COLS; x++) {
            framebuffer[y][x] = GRASS;
        }
    }

    // Draw game objects into framebuffer
    game_draw(&game);
}

int main() {
    setup();

    last_time_ms = to_ms_since_boot(get_absolute_time());

    // Main game loop: input -> update -> draw -> display
    while (true) {
        handle_input();
        update_game();
        render_game();  // fills framebuffer
        render();       // your fixed matrix.c: renders ONE frame
        sleep_ms(33);   // ~30 FPS
    }

    return 0;
}

/*
===============================================================================
ALTERNATIVE APPROACH (if you ever move game loop into matrix.c):

typedef void (*FrameCallback)(void);
void render_with_callback(FrameCallback callback);

void game_frame() {
    handle_input();
    update_game();
    render_game();
}

int main() {
    setup();
    last_time_ms = to_ms_since_boot(get_absolute_time());
    render_with_callback(game_frame);
    return 0;
}
*/

/*
Python version = “full PC game prototype”

    Multiple waves with proper progression.

    Map loaded from JSON with decorations and configurable path/slots/waves.

    Special abilities (Apache, bomber).

    Banner plane between rounds.

    Full splitter behavior.

    More UI feedback (tower ranges, wave info, etc.).

C++ version (now) = “core mechanics port for embedded hardware”

Same basic enemy + tower types and rules.

Same idea of invisible/ghost enemies + radar towers.

Same concept of projectiles, splash damage, money/lives.

Simpler spawn logic (continuous, not wave-based).

No abilities, no banner plane, no JSON map loading, no split-child spawning yet.

Much simpler visuals and controls.*/
