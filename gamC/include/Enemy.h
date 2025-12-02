PSEUDOCODE Enemy.h:

ENUM EnemyType:
    ENEMY_SCOUT
    ENEMY_TANK
    ENEMY_SPLITTER
    ENEMY_GHOST

STRUCT EnemyStats:
    uint8_t health
    uint8_t speed           // pixels per second
    Color color
    Color armor_color       // Optional
    uint8_t reward
    uint8_t damage
    bool invisible
    bool splits_on_death
    uint8_t split_count
    EnemyType split_type

CLASS Enemy:
    PRIVATE:
        EnemyType type
        EnemyStats stats
        
        // Position (use fixed-point for sub-pixel accuracy)
        fixed_point_t x_fixed, y_fixed
        
        // Path following
        const Point* path              // Pointer to path array
        uint8_t path_length
        uint8_t current_path_index
        float path_progress            // Total distance traveled
        
        // State
        uint8_t current_health
        bool is_alive
        bool is_revealed               // For radar/stealth
        float reveal_timer
        
        // Timing
        uint32_t spawn_time_ms
        
    PUBLIC:
        CONSTRUCTOR Enemy(type, path, path_length, spawn_time)
        
        STATIC FUNCTION get_stats(type) -> EnemyStats:
            SWITCH type:
                CASE ENEMY_SCOUT:
                    RETURN EnemyStats{health=3, speed=4, color=RED, ...}
                CASE ENEMY_TANK:
                    RETURN EnemyStats{health=10, speed=2, color=GRAY, ...}
                CASE ENEMY_SPLITTER:
                    RETURN EnemyStats{health=2, speed=7, color=ORANGE, ...}
                CASE ENEMY_GHOST:
                    RETURN EnemyStats{health=4, speed=5, color=PURPLE, invisible=true, ...}
        
        FUNCTION update(dt_ms) -> bool:
            // Returns true if reached end of path
            
            // Update reveal timer
            IF reveal_timer > 0:
                reveal_timer -= dt_ms / 1000.0
                IF reveal_timer <= 0:
                    is_revealed = false
            
            IF current_path_index >= path_length - 1:
                RETURN true  // Reached end
            
            // Get target waypoint
            target = path[current_path_index + 1]
            
            // Calculate direction
            current_x = fixed_to_float(x_fixed)
            current_y = fixed_to_float(y_fixed)
            
            dx = target.x - current_x
            dy = target.y - current_y
            distance = sqrt(dx*dx + dy*dy)
            
            IF distance < 0.01:
                // Reached waypoint, move to next
                current_path_index++
                RETURN false
            
            // Move toward target
            move_distance = stats.speed * (dt_ms / 1000.0)
            
            IF distance <= move_distance:
                // Snap to waypoint
                x_fixed = float_to_fixed(target.x)
                y_fixed = float_to_fixed(target.y)
                path_progress += distance
                current_path_index++
            ELSE:
                // Move partial distance
                ratio = move_distance / distance
                x_fixed += float_to_fixed(dx * ratio)
                y_fixed += float_to_fixed(dy * ratio)
                path_progress += move_distance
            
            RETURN false
        
        FUNCTION take_damage(damage) -> void:
            current_health -= damage
            IF current_health <= 0:
                is_alive = false
        
        FUNCTION reveal(duration_seconds) -> void:
            is_revealed = true
            reveal_timer = duration_seconds
        
        FUNCTION is_visible_to_tower(tower) -> bool:
            IF NOT stats.invisible:
                RETURN true
            IF is_revealed:
                RETURN true
            RETURN tower.can_see_invisible
        
        FUNCTION draw(matrix) -> void:
            x = (int16_t)fixed_to_float(x_fixed)
            y = (int16_t)fixed_to_float(y_fixed)
            
            // Ghost refraction effect
            IF stats.invisible AND NOT is_revealed:
                bg_color = matrix.get_pixel(x, y)
                // Add subtle blue tint
                refract_color = Color(
                    min(255, bg_color.r + 3),
                    min(255, bg_color.g + 5),
                    min(255, bg_color.b + 15)
                )
                matrix.set_pixel(x, y, refract_color)
            ELSE:
                matrix.set_pixel(x, y, stats.color)
        
        // Getters
        FUNCTION get_x() -> float
        FUNCTION get_y() -> float
        FUNCTION get_type() -> EnemyType
        FUNCTION alive() -> bool
        FUNCTION get_reward() -> uint8_t