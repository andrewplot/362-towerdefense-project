#include <iostream>
#include <chrono>
#include <thread>

#include "../game_types.h"

// This provides the storage for the extern framebuffer declared in
// lib/matrix/matrix.h (and used in game.cpp).
Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

// Very basic ASCII renderer: just shows enemies/towers as characters
static void dump_ascii_frame(const GameState& game) {
    // We'll render a tiny downscaled view: mark enemies and towers

    for (int y = 0; y < MATRIX_HEIGHT; ++y) {
        for (int x = 0; x < MATRIX_WIDTH; ++x) {
            // Default background
            char ch = '.';

            // Check if any enemy is on this cell
            for (int i = 0; i < game.enemy_count; ++i) {
                int ex = (int)game.enemies[i].x;
                int ey = (int)game.enemies[i].y;
                if (game.enemies[i].alive && ex == x && ey == y) {
                    ch = 'E';
                    break;
                }
            }

            // Check towers
            for (int i = 0; i < game.tower_count; ++i) {
                int tx = (int)game.towers[i].x;
                int ty = (int)game.towers[i].y;
                if (tx == x && ty == y) {
                    ch = 'T';
                    break;
                }
            }

            std::cout << ch;
        }
        std::cout << "\n";
    }

    std::cout << "Enemies: " << game.enemy_count
              << "  Towers: " << game.tower_count
              << "  Money: " << game.money
              << "  Lives: " << game.lives
              << "\n----------------------------------------\n";
}

int main() {
    GameState game{};
    game_init(&game);

    // Spawn an initial enemy so something moves
    game_spawn_enemy(&game, ENEMY_SCOUT);

    // For testing: a fixed timestep
    const float dt = 0.1f; // 100 ms per frame

    for (int frame = 0; frame < 50; ++frame) {
        // Update game logic
        game_update(&game, dt);

        // Clear framebuffer each frame (simulate GRASS background)
        for (int y = 0; y < MATRIX_HEIGHT; ++y) {
            for (int x = 0; x < MATRIX_WIDTH; ++x) {
                framebuffer[y][x] = Color{0, 50, 0};
            }
        }

        // Let the game draw into framebuffer
        game_draw(&game);

        // Dump a crude ASCII representation to the terminal
        std::cout << "Frame " << frame << "\n";
        dump_ascii_frame(game);

        // Slow it down a bit so you can watch it
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
