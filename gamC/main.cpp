PSEUDOCODE main.cpp:

FUNCTION main() -> int:
    // Initialize hardware
    init_system()
    init_gpio()
    init_spi()
    
    // Create LED matrix
    matrix = LEDMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIXEL_BRIGHTNESS)
    IF NOT matrix.init():
        error("Failed to initialize LED matrix")
        RETURN -1
    
    // Load map (hardcoded for embedded)
    map_data = MapLoader::load_hardcoded_forest_map()
    
    // Create game
    game = Game(&matrix, map_data)
    IF NOT game.init():
        error("Failed to initialize game")
        RETURN -1
    
    // Timing
    last_time_ms = get_system_time_ms()
    
    // Main game loop
    WHILE true:
        current_time_ms = get_system_time_ms()
        dt_ms = current_time_ms - last_time_ms
        last_time_ms = current_time_ms
        
        // Cap delta time to prevent huge jumps
        IF dt_ms > 100:
            dt_ms = 100
        
        // Handle inputs (stub for now)
        handle_input(&game)
        
        // Update game logic
        game.update(dt_ms)
        
        // Draw everything
        game.draw()
        
        // Frame rate limiting
        sleep_until_next_frame(current_time_ms)
    
    RETURN 0

FUNCTION handle_input(game) -> void:
    // This will be replaced with actual peripheral handling
    
    // Check for RFID tag scan (tower selection)
    IF rfid_reader.has_new_tag():
        tag_id = rfid_reader.read_tag()
        
        SWITCH tag_id:
            CASE TOWER_1_TAG_ID:
                game->handle_tower_select(TOWER_MACHINE_GUN)
            CASE TOWER_2_TAG_ID:
                game->handle_tower_select(TOWER_CANNON)
            CASE TOWER_3_TAG_ID:
                game->handle_tower_select(TOWER_SNIPER)
            CASE TOWER_4_TAG_ID:
                game->handle_tower_select(TOWER_RADAR)
    
    // Check for controller input
    IF controller.button_pressed(BUTTON_A):
        // Place tower at cursor position
        cursor_pos = get_cursor_position()
        game->handle_click(cursor_pos.x, cursor_pos.y)
    
    IF controller.button_pressed(BUTTON_START):
        game->handle_wave_start()
    
    IF controller.button_pressed(BUTTON_X):
        game->activate_ability(ABILITY_APACHE)
    
    IF controller.button_pressed(BUTTON_Y):
        game->activate_ability(ABILITY_BOMBER)
    
    // Controller D-pad moves cursor
    IF controller.dpad_pressed(DPAD_UP):
        move_cursor(0, -1)
    // ... etc

FUNCTION sleep_until_next_frame(start_time_ms) -> void:
    elapsed = get_system_time_ms() - start_time_ms
    sleep_time = FRAME_TIME_MS - elapsed
    
    IF sleep_time > 0:
        sleep_ms(sleep_time)