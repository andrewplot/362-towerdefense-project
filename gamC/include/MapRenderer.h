PSEUDOCODE MapRenderer.h:

CLASS MapRenderer:
    PRIVATE:
        LEDMatrix* matrix
        MapData* map_data
        
        // Cached background (avoid recalculating every frame)
        Color background_cache[MATRIX_HEIGHT][MATRIX_WIDTH]
        bool background_cached
        
    PUBLIC:
        CONSTRUCTOR MapRenderer(matrix, map_data)
        
        FUNCTION generate_background() -> void:
            // Create textured background once
            FOR y = 0 TO MATRIX_HEIGHT - 1:
                FOR x = 0 TO MATRIX_WIDTH - 1:
                    // Add random variation
                    variation = (rand() % 11) - 5  // -5 to +5
                    
                    r = clamp(map_data->background_color.r + variation, 0, 255)
                    g = clamp(map_data->background_color.g + variation, 0, 255)
                    b = clamp(map_data->background_color.b + variation, 0, 255)
                    
                    background_cache[y][x] = Color(r, g, b)
            
            background_cached = true
        
        FUNCTION draw_background() -> void:
            IF NOT background_cached:
                generate_background()
            
            FOR y = 0 TO MATRIX_HEIGHT - 1:
                FOR x = 0 TO MATRIX_WIDTH - 1:
                    matrix->set_pixel(x, y, background_cache[y][x])
        
        FUNCTION draw_path() -> void:
            path_color = Color(100, 100, 100)
            
            // Draw 3-pixel wide path
            FOR i = 0 TO map_data->path_length - 2:
                p1 = map_data->path[i]
                p2 = map_data->path[i + 1]
                
                // Draw thick line (3 pixels wide)
                draw_thick_line(p1.x, p1.y, p2.x, p2.y, 3, path_color)
        
        FUNCTION draw_thick_line(x1, y1, x2, y2, thickness, color) -> void:
            // Draw center line
            matrix->draw_line(x1, y1, x2, y2, color)
            
            // Draw parallel offset lines for thickness
            IF y1 == y2:  // Horizontal
                matrix->draw_line(x1, y1 - 1, x2, y2 - 1, color)
                matrix->draw_line(x1, y1 + 1, x2, y2 + 1, color)
            ELSE:  // Vertical
                matrix->draw_line(x1 - 1, y1, x2 - 1, y2, color)
                matrix->draw_line(x1 + 1, y1, x2 + 1, y2, color)
        
        FUNCTION draw_tower_slots() -> void:
            slot_color = Color(128, 107, 0)
            
            FOR i = 0 TO map_data->tower_slot_count - 1:
                slot = map_data->tower_slots[i]
                matrix->fill_rect(slot.x - 2, slot.y - 2, 4, 4, slot_color)
        
        FUNCTION draw_all() -> void:
            draw_background()
            draw_path()
            draw_tower_slots()