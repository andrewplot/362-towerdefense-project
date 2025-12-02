PSEUDOCODE Wave.h:

STRUCT WaveDefinition:
    EnemyType enemy_types[30]
    uint8_t enemy_count
    uint16_t spawn_interval_ms
    float reward_multiplier

ENUM WaveState:
    WAVE_WAITING
    WAVE_ACTIVE
    WAVE_COMPLETE
    WAVE_GAME_WON

CLASS WaveManager:
    PRIVATE:
        // Wave definitions (loaded from map)
        WaveDefinition waves[10]
        uint8_t total_wave_count
        uint8_t current_wave_index
        
        // Current wave state
        uint8_t current_spawn_index
        uint16_t time_since_last_spawn_ms
        
        // Inter-wave timing
        uint16_t inter_wave_delay_ms
        uint16_t time_until_next_wave_ms
        
        // State
        WaveState state
        bool auto_start
        
    PUBLIC:
        CONSTRUCTOR WaveManager(wave_defs, wave_count)
        
        FUNCTION start_next_wave() -> bool:
            IF state == WAVE_GAME_WON:
                RETURN false
            
            current_wave_index++
            
            IF current_wave_index >= total_wave_count:
                state = WAVE_GAME_WON
                RETURN false
            
            current_spawn_index = 0
            time_since_last_spawn_ms = 0
            state = WAVE_ACTIVE
            RETURN true
        
        FUNCTION update(dt_ms, game) -> void:
            IF state == WAVE_WAITING:
                // Waiting for player to start
                RETURN
            
            ELSE IF state == WAVE_ACTIVE:
                time_since_last_spawn_ms += dt_ms
                
                current_wave = &waves[current_wave_index]
                
                // Time to spawn next enemy?
                IF time_since_last_spawn_ms >= current_wave->spawn_interval_ms:
                    IF current_spawn_index < current_wave->enemy_count:
                        enemy_type = current_wave->enemy_types[current_spawn_index]
                        game->spawn_enemy(enemy_type)
                        current_spawn_index++
                        time_since_last_spawn_ms = 0
                
                // Check if wave complete
                IF current_spawn_index >= current_wave->enemy_count:
                    IF game->get_enemy_count() == 0:
                        complete_wave(game)
            
            ELSE IF state == WAVE_COMPLETE:
                time_until_next_wave_ms -= dt_ms
                
                IF time_until_next_wave_ms <= 0 AND auto_start:
                    start_next_wave()
        
        FUNCTION complete_wave(game) -> void:
            state = WAVE_COMPLETE
            time_until_next_wave_ms = inter_wave_delay_ms
            
            // Award bonus money
            bonus = 20 + (current_wave_index * 10)
            game->add_money(bonus)
        
        FUNCTION manual_start() -> bool:
            IF state == WAVE_WAITING OR state == WAVE_COMPLETE:
                RETURN start_next_wave()
            RETURN false
        
        // Getters
        FUNCTION get_current_wave() -> uint8_t
        FUNCTION get_total_waves() -> uint8_t
        FUNCTION get_state() -> WaveState