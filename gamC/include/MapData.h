PSEUDOCODE MapData.h:

STRUCT MapData:
    // Map info
    char name[32]
    uint8_t width, height
    Color background_color
    
    // Path (fixed-size arrays for embedded)
    Point path[MAX_PATH_POINTS]
    uint8_t path_length
    
    // Tower placement
    Point tower_slots[MAX_TOWER_SLOTS]
    uint8_t tower_slot_count
    
    // Wave definitions
    WaveDefinition waves[10]
    uint8_t wave_count

CLASS MapLoader:
    PUBLIC:
        STATIC FUNCTION load_hardcoded_forest_map() -> MapData:
            // Since we're on embedded, hardcode the map instead of JSON
            map = MapData()
            
            strcpy(map.name, "Forest")
            map.width = 64
            map.height = 32
            map.background_color = Color(0, 60, 0)
            
            // Define path
            map.path[0] = Point(63, 20)
            map.path[1] = Point(20, 20)
            map.path[2] = Point(20, 10)
            map.path[3] = Point(0, 10)
            map.path_length = 4
            
            // Define tower slots
            map.tower_slots[0] = Point(10, 5)
            map.tower_slots[1] = Point(10, 25)
            map.tower_slots[2] = Point(40, 15)
            map.tower_slots[3] = Point(50, 10)
            map.tower_slots[4] = Point(50, 26)
            map.tower_slot_count = 5
            
            // Define waves
            map.waves[0] = create_wave_1()
            map.waves[1] = create_wave_2()
            // ... etc
            map.wave_count = 7
            
            RETURN map
        
        STATIC FUNCTION create_wave_1() -> WaveDefinition:
            wave = WaveDefinition()
            wave.enemy_types[0] = ENEMY_SCOUT
            wave.enemy_types[1] = ENEMY_SCOUT
            wave.enemy_types[2] = ENEMY_SCOUT
            wave.enemy_types[3] = ENEMY_SCOUT
            wave.enemy_types[4] = ENEMY_SCOUT
            wave.enemy_count = 5
            wave.spawn_interval_ms = 1500
            wave.reward_multiplier = 1.0
            RETURN wave