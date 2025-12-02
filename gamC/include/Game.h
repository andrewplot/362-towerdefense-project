PSEUDOCODE Game.h:

CLASS Game:
    PRIVATE:
        // Core systems
        LEDMatrix* matrix
        MapData map_data
        MapRenderer renderer
        WaveManager wave_manager
        AbilityManager ability_manager
        
        // Game entities (fixed-size pools)
        Enemy enemies[MAX_ENEMIES]
        uint8_t active_enemy_count
        
        Tower towers[MAX_TOWERS]
        uint8_t active_tower_count
        
        // Game state
        uint16_t money
        uint8_t lives
        uint16_t score
        uint32_t game_time_ms
        bool game_over
        bool game_won
        
        // UI state
        TowerType selected_tower_type
        bool show_tower_ranges
        
    PUBLIC:
        CONSTRUCTOR Game(matrix, map_data)
        
        FUNCTION init() -> bool:
            money = STARTING_MONEY
            lives = STARTING_LIVES
            score = 0
            game_time_ms = 0
            game_over = false
            game_won = false
            
            active_enemy_count = 0
            active_tower_count = 0
            
            renderer.generate_background()
            
            RETURN true
        
        FUNCTION spawn_enemy(type) -> bool:
            IF active_enemy_count >= MAX_ENEMIES:
                RETURN false
            
            enemies[active_enemy_count] = Enemy(
                type, 
                map_data.path, 
                map_data.path_length,
                game_time_ms
            )
            active_enemy_count++
            RETURN true
        
        FUNCTION place_tower(type, x, y) -> bool:
            // Check cost
            cost = Tower::get_cost(type)
            IF money < cost:
                RETURN false
            
            // Find nearest tower slot
            nearest_slot = find_nearest_tower_slot(x, y)
            IF nearest_slot == NULL:
                RETURN false
            
            // Check if slot occupied
            FOR i = 0 TO active_tower_count - 1:
                IF towers[i].get_x() == nearest_slot->x AND 
                   towers[i].get_y() == nearest_slot->y:
                    RETURN false
            
            // Place tower
            IF active_tower_count >= MAX_TOWERS:
                RETURN false
            
            towers[active_tower_count] = Tower(type, nearest_slot->x, nearest_slot->y)
            active_tower_count++
            money -= cost
            RETURN true
        
        FUNCTION find_nearest_tower_slot(x, y) -> Point*:
            nearest = NULL
            nearest_dist = 999.9
            
            FOR i = 0 TO map_data.tower_slot_count - 1:
                slot = &map_data.tower_slots[i]
                dist = calculate_distance(x, y, slot->x, slot->y)
                
                IF dist < nearest_dist AND dist <= 5:
                    nearest_dist = dist
                    nearest = slot
            
            RETURN nearest
        
        FUNCTION activate_ability(type) -> bool:
            IF NOT ability_manager.can_activate(type, money, game_time_ms):
                RETURN false
            
            // Deduct cost
            cost = (type == ABILITY_APACHE) ? 50 : 75
            money -= cost
            
            // Calculate average path Y
            path_y = 0
            FOR i = 0 TO map_data.path_length - 1:
                path_y += map_data.path[i].y
            path_y /= map_data.path_length
            
            RETURN ability_manager.activate(type, game_time_ms, path_y)
        
        FUNCTION update(dt_ms) -> void:
            IF game_over OR game_won:
                RETURN
            
            game_time_ms += dt_ms
            
            // Update wave system
            wave_manager.update(dt_ms, this)
            
            // Check win condition
            IF wave_manager.get_state() == WAVE_GAME_WON:
                game_won = true
                RETURN
            
            // Update abilities
            ability_manager.update(dt_ms, enemies, active_enemy_count)
            
            // Update towers
            FOR i = 0 TO active_tower_count - 1:
                towers[i].update(dt_ms, enemies, active_enemy_count)
            
            // Update enemies
            FOR i = 0 TO active_enemy_count - 1:
                enemy = &enemies[i]
                
                reached_end = enemy->update(dt_ms)
                
                IF reached_end:
                    // Enemy reached end
                    lives -= enemy->get_damage()
                    IF lives <= 0:
                        lives = 0
                        game_over = true
                    
                    // Remove enemy (swap with last)
                    enemies[i] = enemies[active_enemy_count - 1]
                    active_enemy_count--
                    i--  // Recheck this index
                    CONTINUE
                
                IF NOT enemy->alive():
                    // Enemy died
                    money += enemy->get_reward()
                    score += enemy->get_reward()
                    
                    // Handle splitters
                    IF enemy->should_split():
                        spawn_splits(enemy)
                    
                    // Remove enemy
                    enemies[i] = enemies[active_enemy_count - 1]
                    active_enemy_count--
                    i--
        
        FUNCTION spawn_splits(parent_enemy) -> void:
            split_type = parent_enemy->get_split_type()
            split_count = parent_enemy->get_split_count()
            
            FOR i = 0 TO split_count - 1:
                IF active_enemy_count >= MAX_ENEMIES:
                    BREAK
                
                // Spawn offset from parent
                offset_x = (i - split_count / 2.0) * 2
                
                enemies[active_enemy_count] = Enemy(
                    split_type,
                    map_data.path,
                    map_data.path_length,
                    game_time_ms
                )
                
                // Set position to parent's position with offset
                enemies[active_enemy_count].set_position(
                    parent_enemy->get_x() + offset_x,
                    parent_enemy->get_y()
                )
                
                // Find closest path point
                enemies[active_enemy_count].find_closest_path_point()
                
                active_enemy_count++
        
        FUNCTION draw() -> void:
            matrix->clear()
            
            // Draw map
            renderer.draw_all()
            
            // Draw tower ranges if enabled
            IF show_tower_ranges:
                FOR i = 0 TO active_tower_count - 1:
                    towers[i].draw_range(matrix)
            
            // Draw towers
            FOR i = 0 TO active_tower_count - 1:
                towers[i].draw(matrix)
            
            // Draw enemies
            FOR i = 0 TO active_enemy_count - 1:
                enemies[i].draw(matrix)
            
            // Draw abilities (on top)
            ability_manager.draw(matrix)
            
            // Update display
            matrix->update_display()
        
        // Input handling
        FUNCTION handle_tower_select(type) -> void:
            selected_tower_type = type
        
        FUNCTION handle_click(x, y) -> bool:
            IF selected_tower_type != NONE:
                RETURN place_tower(selected_tower_type, x, y)
            RETURN false
        
        FUNCTION handle_wave_start() -> bool:
            RETURN wave_manager.manual_start()
        
        // Getters
        FUNCTION get_money() -> uint16_t
        FUNCTION get_lives() -> uint8_t
        FUNCTION get_score() -> uint16_t
        FUNCTION is_game_over() -> bool
        FUNCTION is_game_won() -> bool