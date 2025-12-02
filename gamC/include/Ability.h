PSEUDOCODE Ability.h:

ENUM AbilityType:
    ABILITY_APACHE
    ABILITY_BOMBER

CLASS Bomb:
    PRIVATE:
        fixed_point_t x_fixed, y_fixed
        uint8_t damage
        uint8_t splash_radius
        int16_t path_y              // Y coordinate to explode at
        uint8_t fall_speed
        bool active
        bool exploded
        float explosion_timer
        
    PUBLIC:
        CONSTRUCTOR Bomb(x, y, damage, splash, path_y)
        
        FUNCTION update(dt_ms, enemies, enemy_count) -> void:
            IF exploded:
                explosion_timer += dt_ms / 1000.0
                IF explosion_timer > 0.15:
                    active = false
                RETURN
            
            // Fall down
            y_fixed += float_to_fixed(fall_speed * dt_ms / 1000.0)
            
            IF fixed_to_float(y_fixed) >= path_y + 2:
                explode(enemies, enemy_count)
        
        FUNCTION explode(enemies, enemy_count) -> void:
            exploded = true
            
            x = fixed_to_float(x_fixed)
            y = fixed_to_float(y_fixed)
            
            FOR i = 0 TO enemy_count - 1:
                enemy = &enemies[i]
                IF NOT enemy->alive():
                    CONTINUE
                
                distance = calculate_distance(x, y, enemy->get_x(), enemy->get_y())
                IF distance <= splash_radius:
                    enemy->take_damage(damage)
        
        FUNCTION draw(matrix) -> void:
            x = (int16_t)fixed_to_float(x_fixed)
            y = (int16_t)fixed_to_float(y_fixed)
            
            IF NOT exploded:
                matrix.fill_rect(x, y, 2, 2, Color(255, 200, 0))
            ELSE:
                // Draw explosion
                size = (int16_t)(splash_radius * 1.5)
                matrix.fill_rect(
                    x - size/2, y - size/2, size, size,
                    Color(255, 100, 0)
                )
                matrix.fill_rect(x - 1, y - 1, 2, 2, Color(255, 255, 0))

CLASS ApacheStrike:
    PRIVATE:
        fixed_point_t x_fixed, y_fixed
        int8_t speed                // Negative (moving left)
        uint32_t spawn_time_ms
        uint32_t game_time_ms
        
        // Combat
        uint8_t damage
        float fire_rate
        uint8_t range
        float time_since_last_shot
        
        // Projectiles
        Projectile projectiles[16]
        uint8_t active_projectile_count
        
        bool active
        
    PUBLIC:
        CONSTRUCTOR ApacheStrike(start_x, start_y, game_time)
        
        FUNCTION update(dt_ms, enemies, enemy_count) -> void:
            game_time_ms += dt_ms
            
            // Move left
            x_fixed -= float_to_fixed(speed * dt_ms / 1000.0)
            
            // Check if off-screen
            IF fixed_to_float(x_fixed) < -5:
                active = false
                RETURN
            
            // Update shooting
            time_since_last_shot += dt_ms / 1000.0
            
            // Update projectiles
            FOR i = 0 TO active_projectile_count - 1:
                IF projectiles[i].update(dt_ms, enemies, enemy_count):
                    projectiles[i] = projectiles[active_projectile_count - 1]
                    active_projectile_count--
                    i--
            
            // Try to shoot
            IF time_since_last_shot >= fire_rate:
                target = find_target(enemies, enemy_count)
                IF target != NULL:
                    shoot(target)
                    time_since_last_shot = 0
        
        FUNCTION draw(matrix) -> void:
            x = (int16_t)fixed_to_float(x_fixed)
            y = (int16_t)fixed_to_float(y_fixed)
            
            // Draw helicopter body (detailed in Python version)
            // Main body
            matrix.fill_rect(x - 1, y - 1, 3, 3, Color(50, 150, 50))
            
            // Tail
            matrix.fill_rect(x + 2, y, 2, 1, Color(40, 120, 40))
            
            // Animated rotors based on game_time_ms
            rotor_phase = (game_time_ms * 12 / 1000) % 4
            draw_rotor_blades(matrix, x, y, rotor_phase)
            
            // Draw projectiles
            FOR i = 0 TO active_projectile_count - 1:
                projectiles[i].draw(matrix)

CLASS BomberStrike:
    PRIVATE:
        fixed_point_t x_fixed, y_fixed
        int8_t speed_x              // Negative (moving left)
        int16_t path_y              // Target Y for bombing
        
        uint8_t damage
        uint8_t splash_radius
        float drop_interval
        float time_since_last_drop
        
        Bomb bombs[8]
        uint8_t active_bomb_count
        
        bool active
        
    PUBLIC:
        CONSTRUCTOR BomberStrike(start_x, start_y, path_y)
        
        FUNCTION update(dt_ms, enemies, enemy_count) -> void:
            // Move left
            x_fixed += float_to_fixed(speed_x * dt_ms / 1000.0)
            
            IF fixed_to_float(x_fixed) < -10:
                active = false
                RETURN
            
            // Update drop timer
            time_since_last_drop += dt_ms / 1000.0
            
            // Update bombs
            FOR i = 0 TO active_bomb_count - 1:
                bombs[i].update(dt_ms, enemies, enemy_count)
                IF NOT bombs[i].active:
                    bombs[i] = bombs[active_bomb_count - 1]
                    active_bomb_count--
                    i--
            
            // Drop bombs
            IF time_since_last_drop >= drop_interval:
                drop_bomb()
                time_since_last_drop = 0
        
        FUNCTION drop_bomb() -> void:
            IF active_bomb_count >= 8:
                RETURN
            
            x = fixed_to_float(x_fixed)
            y = fixed_to_float(y_fixed) + 3
            
            bombs[active_bomb_count] = Bomb(x, y, damage, splash_radius, path_y)
            active_bomb_count++
        
        FUNCTION draw(matrix) -> void:
            // Draw bomber plane (detailed design from Python)
            x = (int16_t)fixed_to_float(x_fixed)
            y = (int16_t)fixed_to_float(y_fixed)
            
            // Wings, fuselage, engines, etc.
            matrix.fill_rect(x, y - 5, 2, 10, Color(100, 100, 100))
            matrix.fill_rect(x - 2, y - 1, 6, 2, Color(120, 120, 120))
            
            // Draw bombs
            FOR i = 0 TO active_bomb_count - 1:
                bombs[i].draw(matrix)

CLASS AbilityManager:
    PRIVATE:
        // Active abilities pool
        ApacheStrike apaches[2]
        uint8_t active_apache_count
        
        BomberStrike bombers[2]
        uint8_t active_bomber_count
        
        // Cooldown tracking
        uint32_t apache_cooldown_end_ms
        uint32_t bomber_cooldown_end_ms
        
    PUBLIC:
        CONSTRUCTOR AbilityManager()
        
        FUNCTION can_activate(type, money, current_time_ms) -> bool:
            cost = get_ability_cost(type)
            IF money < cost:
                RETURN false
            
            IF type == ABILITY_APACHE:
                RETURN current_time_ms >= apache_cooldown_end_ms
            ELSE IF type == ABILITY_BOMBER:
                RETURN current_time_ms >= bomber_cooldown_end_ms
            
            RETURN false
        
        FUNCTION activate(type, current_time_ms, path_y) -> bool:
            IF type == ABILITY_APACHE:
                IF active_apache_count >= 2:
                    RETURN false
                
                apaches[active_apache_count] = ApacheStrike(64 + 5, 5, current_time_ms)
                active_apache_count++
                apache_cooldown_end_ms = current_time_ms + 15000  // 15 sec cooldown
                RETURN true
                
            ELSE IF type == ABILITY_BOMBER:
                IF active_bomber_count >= 2:
                    RETURN false
                
                bombers[active_bomber_count] = BomberStrike(64 + 5, 8, path_y)
                active_bomber_count++
                bomber_cooldown_end_ms = current_time_ms + 20000  // 20 sec cooldown
                RETURN true
            
            RETURN false
        
        FUNCTION update(dt_ms, enemies, enemy_count) -> void:
            // Update apaches
            FOR i = 0 TO active_apache_count - 1:
                apaches[i].update(dt_ms, enemies, enemy_count)
                IF NOT apaches[i].active:
                    apaches[i] = apaches[active_apache_count - 1]
                    active_apache_count--
                    i--
            
            // Update bombers
            FOR i = 0 TO active_bomber_count - 1:
                bombers[i].update(dt_ms, enemies, enemy_count)
                IF NOT bombers[i].active:
                    bombers[i] = bombers[active_bomber_count - 1]
                    active_bomber_count--
                    i--
        
        FUNCTION draw(matrix) -> void:
            FOR i = 0 TO active_apache_count - 1:
                apaches[i].draw(matrix)
            FOR i = 0 TO
                active_bomber_count - 1:
                bombers[i].draw(matrix)