// game_types.h - Core game data structures for tower defense
#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "../lib/color.h"

// ============================================================================
// Configuration constants
// ============================================================================

#define MAX_ENEMIES         50
#define MAX_TOWERS          10
#define MAX_PROJECTILES     30
#define MAX_PATH_WAYPOINTS  20

#define MATRIX_WIDTH        64
#define MATRIX_HEIGHT       32

extern Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
// ============================================================================
// Enemy system
// ============================================================================

typedef enum {
    ENEMY_SCOUT = 0,
    ENEMY_TANK,
    ENEMY_SPLITTER,
    ENEMY_GHOST
} EnemyType;

typedef struct {
    int      health;           // Hit points
    float    speed;            // Tiles per second
    Color    color;            // Display color
    uint8_t  reward;           // Money given on kill
    uint8_t  damage;           // Lives lost on leak
    bool     invisible;        // True if requires radar to see
    bool     splits_on_death;  // For splitter enemies
    uint8_t  split_count;      // Number of children to spawn
} EnemyStats;

typedef struct {
    // Position & movement
    float     x;
    float     y;
    float     speed;           // Movement speed along path

    // Health
    int       health;
    int       max_health;

    // Identity / visuals
    EnemyType type;
    Color     color;

    // Path following
    uint8_t   path_index;      // Current waypoint index
    float     path_progress;   // Total distance travelled along path

    // State flags
    bool      alive;
    bool      invisible;       // For ghost enemies
    bool      revealed;        // If radar tower has revealed it
} Enemy;

// ============================================================================
// Tower system
// ============================================================================

typedef enum {
    TOWER_MACHINE_GUN = 0,
    TOWER_CANNON,
    TOWER_SNIPER,
    TOWER_RADAR
} TowerType;

typedef struct {
    // Position
    float     x;
    float     y;

    TowerType type;            // Tower type
    Color     color;           // Display color

    // Combat stats
    uint8_t   damage;
    float     range;
    float     fire_rate;       // Shots per second
    float     projectile_speed;
    uint8_t   splash_radius;   // 0 = no splash, otherwise AoE radius

    // Firing state
    float     time_since_shot; // Time since last shot
    int8_t    target_index;    // Index of current target (-1 = none)

    // Special abilities
    bool      can_see_invisible;
    bool      is_radar;
    float     radar_angle;     // For radar sweep animation
} Tower;

typedef struct {
    uint8_t   cost;
    uint8_t   damage;
    float     range;
    float     fire_rate;       // Shots per second
    float     projectile_speed;
    Color     color;
    bool      can_see_invisible;
    bool      is_radar;
    uint8_t   splash_radius;
} TowerStats;

// ============================================================================
// Projectile system
// ============================================================================

typedef struct {
    float     x;
    float     y;
    uint8_t   target_index;    // Index of target enemy
    uint8_t   damage;
    float     speed;           // 0 = instant hit (unused currently)
    Color     color;
    uint8_t   splash_radius;   // 0 = no splash
    bool      active;
} Projectile;

// ============================================================================
// Wave system
// ============================================================================

typedef struct {
    EnemyType enemies[30];     // Enemy types to spawn
    uint8_t   enemy_count;     // Total enemies in wave
    uint8_t   spawn_index;     // Next enemy to spawn
    float     spawn_interval;  // Seconds between spawns
    float     time_since_spawn;// Timer since last spawn
    bool      completed;       // All enemies spawned
} Wave;

// ============================================================================
// Game state
// ============================================================================

typedef struct {
    // Collections
    Enemy      enemies[MAX_ENEMIES];
    uint8_t    enemy_count;

    Tower      towers[MAX_TOWERS];
    uint8_t    tower_count;

    Projectile projectiles[MAX_PROJECTILES];
    uint8_t    projectile_count;

    // Path definition (map)
    struct {
        int16_t x;
        int16_t y;
    } path[MAX_PATH_WAYPOINTS];
    uint8_t    path_length;

    // Tower slots (predefined build locations)
    struct {
        int16_t x;
        int16_t y;
        bool    occupied;
    } tower_slots[MAX_TOWERS];
    uint8_t    tower_slot_count;

    // Game stats
    uint16_t   money;
    uint8_t    lives;
    uint16_t   score;
    float      game_time;

    // Wave management
    Wave       waves[10];
    uint8_t    wave_number;
    uint8_t    total_waves;
    bool       wave_active;

    // Misc
    TowerType  selected_tower;
} GameState;

// ============================================================================
// Function prototypes (implemented in game.cpp)
// ============================================================================

// Enemy functions
void enemy_init(Enemy* enemy, EnemyType type, float start_x, float start_y);
void enemy_update(Enemy* enemy, float dt, GameState* game);
void enemy_draw(const Enemy* enemy);

// Tower functions
void tower_init(Tower* tower, TowerType type, int16_t x, int16_t y);
void tower_update(Tower* tower, float dt, GameState* game);
void tower_draw(const Tower* tower);

// Projectile functions
void projectile_init(Projectile* proj,
                     float x,
                     float y,
                     uint8_t target_idx,
                     uint8_t damage,
                     float speed,
                     Color color,
                     uint8_t splash);
bool projectile_update(Projectile* proj, float dt, GameState* game);
void projectile_draw(const Projectile* proj);

// Game functions
void game_init(GameState* game);
void game_update(GameState* game, float dt);
void game_draw(const GameState* game);
bool game_place_tower(GameState* game, TowerType type, int16_t x, int16_t y);
void game_spawn_enemy(GameState* game, EnemyType type);
void game_start_wave(GameState* game);

// Utility functions
float distance_squared(float x1, float y1, float x2, float y2);
float distance(float x1, float y1, float x2, float y2);
bool is_in_range(float x1, float y1, float x2, float y2, float range);

#endif // GAME_TYPES_H
