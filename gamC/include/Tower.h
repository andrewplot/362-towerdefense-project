PSEUDOCODE Tower.h:

ENUM TowerType:
    TOWER_MACHINE_GUN
    TOWER_CANNON
    TOWER_SNIPER
    TOWER_RADAR

STRUCT TowerStats:
    uint8_t damage
    uint8_t range
    float fire_rate              // Seconds between shots
    uint8_t projectile_speed     // 0 = instant hit
    Color color
    Color projectile_color
    bool can_see_invisible
    uint8_t splash_radius        // 0 = no splash
    bool is_radar

CLASS Projectile:
    PRIVATE:
        fixed_point_t x_fixed, y_fixed
        Enemy* target              // Pointer to target enemy
        uint8_t damage
        uint8_t speed
        Color color
        uint8_t splash_radius
        bool active
        
    PUBLIC:
        CONSTRUCTOR Projectile(x, y, target, damage, speed, color, splash)
        
        FUNCTION update(dt_ms, all_enemies, enemy_count) -> bool:
            // Returns true if should be removed
            
            IF NOT target->alive():
                active = false
                RETURN true
            
            // Instant hit (sniper)
            IF speed == 0:
                hit_target(all_enemies, enemy_count)
                RETURN true
            
            // Calculate movement
            target_x = target->get_x()
            target_y = target->get_y()
            current_x = fixed_to_float(x_fixed)
            current_y = fixed_to_float(y_fixed)
            
            dx = target_x - current_x
            dy = target_y - current_y
            distance = sqrt(dx*dx + dy*dy)
            
            move_distance = speed * (dt_ms / 1000.0)
            
            IF distance <= move_distance OR distance < 0.5:
                hit_target(all_enemies, enemy_count)
                RETURN true
            
            // Move toward target
            ratio = move_distance / distance
            x_fixed += float_to_fixed(dx * ratio)
            y_fixed += float_to_fixed(dy * ratio)
            
            RETURN false
        
        FUNCTION hit_target(all_enemies, enemy_count) -> void:
            // Direct damage
            target->take_damage(damage)
            
            // Splash damage
            IF splash_radius > 0:
                FOR i = 0 TO enemy_count - 1:
                    enemy = &all_enemies[i]
                    IF NOT enemy->alive() OR enemy == target:
                        CONTINUE
                    
                    dist = distance_between(enemy, target)
                    IF dist <= splash_radius:
                        enemy->take_damage(damage / 2)  // 50% splash
            
            active = false
        
        FUNCTION draw(matrix) -> void:
            IF active:
                x = (int16_t)fixed_to_float(x_fixed)
                y = (int16_t)fixed_to_float(y_fixed)
                matrix.set_pixel(x, y, color)

CLASS Tower:
    PRIVATE:
        TowerType type
        TowerStats stats
        int16_t x, y
        
        // Combat state
        float time_since_last_shot
        Enemy* current_target
        
        // Projectile pool (avoid dynamic allocation)
        Projectile projectiles[8]
        uint8_t active_projectile_count
        
        // Radar-specific
        float radar_angle           // Radians
        float radar_sweep_speed
        
    PUBLIC:
        CONSTRUCTOR Tower(type, x, y)
        
        STATIC FUNCTION get_stats(type) -> TowerStats:
            SWITCH type:
                CASE TOWER_MACHINE_GUN:
                    RETURN TowerStats{damage=1, range=14, fire_rate=0.5, ...}
                CASE TOWER_CANNON:
                    RETURN TowerStats{damage=2, range=10, fire_rate=1.5, splash=3, ...}
                CASE TOWER_SNIPER:
                    RETURN TowerStats{damage=4, range=26, fire_rate=2.0, speed=0, can_see_invisible=true, ...}
                CASE TOWER_RADAR:
                    RETURN TowerStats{range=22, is_radar=true, can_see_invisible=true, ...}
        
        FUNCTION update(dt_ms, enemies, enemy_count) -> void:
            // Radar towers sweep and reveal
            IF stats.is_radar:
                radar_angle += radar_sweep_speed * (dt_ms / 1000.0)
                IF radar_angle > TWO_PI:
                    radar_angle -= TWO_PI
                
                update_radar_sweep(enemies, enemy_count)
                RETURN
            
            // Update shooting timer
            time_since_last_shot += dt_ms / 1000.0
            
            // Update projectiles
            FOR i = 0 TO active_projectile_count - 1:
                IF projectiles[i].update(dt_ms, enemies, enemy_count):
                    // Remove projectile (swap with last)
                    projectiles[i] = projectiles[active_projectile_count - 1]
                    active_projectile_count--
                    i--  // Recheck this index
            
            // Try to shoot
            IF time_since_last_shot >= stats.fire_rate:
                target = find_target(enemies, enemy_count)
                IF target != NULL:
                    shoot(target)
                    time_since_last_shot = 0
        
        FUNCTION find_target(enemies, enemy_count) -> Enemy*:
            best_target = NULL
            best_progress = -1.0
            
            FOR i = 0 TO enemy_count - 1:
                enemy = &enemies[i]
                
                IF NOT enemy->alive():
                    CONTINUE
                
                IF NOT enemy->is_visible_to_tower(this):
                    CONTINUE
                
                distance = calculate_distance(x, y, enemy->get_x(), enemy->get_y())
                IF distance > stats.range:
                    CONTINUE
                
                // Prioritize furthest along path
                IF enemy->get_path_progress() > best_progress:
                    best_progress = enemy->get_path_progress()
                    best_target = enemy
            
            RETURN best_target
        
        FUNCTION shoot(target) -> void:
            IF active_projectile_count >= 8:
                RETURN  // Projectile pool full
            
            projectiles[active_projectile_count] = Projectile(
                x, y, target, stats.damage, stats.projectile_speed,
                stats.projectile_color, stats.splash_radius
            )
            active_projectile_count++
        
        FUNCTION update_radar_sweep(enemies, enemy_count) -> void:
            sweep_width = 0.3  // Radians
            
            FOR i = 0 TO enemy_count - 1:
                enemy = &enemies[i]
                
                IF NOT enemy->is_invisible():
                    CONTINUE
                
                distance = calculate_distance(x, y, enemy->get_x(), enemy->get_y())
                IF distance > stats.range:
                    CONTINUE
                
                // Check if enemy in sweep arc
                angle_to_enemy = atan2(enemy->get_y() - y, enemy->get_x() - x)
                IF angle_to_enemy < 0:
                    angle_to_enemy += TWO_PI
                
                angle_diff = abs(angle_to_enemy - radar_angle)
                IF angle_diff > PI:
                    angle_diff = TWO_PI - angle_diff
                
                IF angle_diff < sweep_width:
                    enemy->reveal(0.5)  // Reveal for 0.5 seconds
        
        FUNCTION draw(matrix) -> void:
            // Draw radar range circle first (if radar)
            IF stats.is_radar:
                draw_circle_outline(matrix, x, y, stats.range, Color(0, 80, 0))
            
            // Draw tower base
            matrix.fill_rect(x - 1, y - 1, 3, 3, stats.color)
            
            // Draw radar sweep line
            IF stats.is_radar:
                end_x = x + (int16_t)(cos(radar_angle) * (stats.range - 1))
                end_y = y + (int16_t)(sin(radar_angle) * (stats.range - 1))
                matrix.draw_line(x, y, end_x, end_y, Color(0, 120, 0))
            
            // Draw projectiles
            FOR i = 0 TO active_projectile_count - 1:
                projectiles[i].draw(matrix)
        
        FUNCTION draw_range(matrix) -> void:
            draw_circle_outline(matrix, x, y, stats.range, Color(100, 100, 100))
        
        STATIC FUNCTION get_cost(type) -> uint16_t:
            SWITCH type:
                CASE TOWER_MACHINE_GUN: RETURN 30
                CASE TOWER_CANNON: RETURN 50
                CASE TOWER_SNIPER: RETURN 65
                CASE TOWER_RADAR: RETURN 40