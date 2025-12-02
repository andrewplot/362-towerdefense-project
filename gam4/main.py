"""
Main entry point for Tower Defense Game
"""

import pygame
import os
from led_matrix import LEDMatrixSimulator
from map_data import MapLoader, create_sample_maps
from game import Game
from tower import Tower


def setup_maps():
    """Create sample maps if they don't exist"""
    if not os.path.exists("maps"):
        os.makedirs("maps", exist_ok=True)
        maps = create_sample_maps()
        for i, map_data in enumerate(maps, 1):
            MapLoader.save_json(f"maps/map{i}.json", map_data)


def main():
    """Main game loop"""
    
    # Setup
    setup_maps()
    
    # Try to load forest.json, fallback to map1.json
    try:
        map_data = MapLoader.load_json("maps/forest.json")
    except FileNotFoundError:
        map_data = MapLoader.load_json("maps/map1.json")
    
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    game = Game(matrix, map_data)
    
    # Game loop
    clock = pygame.time.Clock()
    running = True
    
    print("\n=== TOWER DEFENSE CONTROLS ===")
    print("\nTOWERS:")
    print("  1 = Machine Gun ($30) - Fast fire, general purpose")
    print("  2 = Cannon ($50) - Splash damage, short range")
    print("  3 = Sniper ($65) - High damage, sees invisible")
    print("  4 = Radar ($40) - Reveals invisible enemies")
    print("\nWAVE CONTROLS:")
    print("  SPACE = Start next wave")
    print("  M = Toggle auto-start waves")
    print("\nMANUAL ENEMY SPAWNING (testing):")
    print("  Q = Scout (3 HP, normal speed)")
    print("  W = Tank (10 HP, slow, 2 damage)")
    print("  E = Splitter (2 HP, fast, splits into 2 scouts)")
    print("  R = Ghost (4 HP, invisible)")
    print("\nABILITIES:")
    print("  A = Apache Strike ($50)")
    print("  B = Bomber Run ($75)")
    print("\nOTHER:")
    print("  T = Toggle tower ranges")
    print("  Click = Place selected tower")
    print("  ESC = Quit")
    print("\n===============================\n")
    
    while running:
        dt = clock.get_time() / 1000.0
        
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                
                # Wave controls
                elif event.key == pygame.K_SPACE:
                    if game.start_wave():
                        wave_info = game.get_wave_info()
                        print(f"Started Wave {wave_info['current']}/{wave_info['total']}")
                    else:
                        print("Cannot start wave (wave already active or game over)")
                
                elif event.key == pygame.K_m:
                    game.wave_manager.auto_start = not game.wave_manager.auto_start
                    print(f"Auto-start waves: {'ON' if game.wave_manager.auto_start else 'OFF'}")
                
                # Tower selection
                elif event.key == pygame.K_1:
                    game.selected_tower_type = "machine_gun"
                    print("Selected: Machine Gun ($30)")
                elif event.key == pygame.K_2:
                    game.selected_tower_type = "cannon"
                    print("Selected: Cannon ($50)")
                elif event.key == pygame.K_3:
                    game.selected_tower_type = "sniper"
                    print("Selected: Sniper ($65)")
                elif event.key == pygame.K_4:
                    game.selected_tower_type = "radar"
                    print("Selected: Radar ($40)")
                
                # Manual enemy spawning (for testing)
                elif event.key == pygame.K_q:
                    game.spawn_enemy("scout")
                    print("Spawned: Scout (manual)")
                elif event.key == pygame.K_w:
                    game.spawn_enemy("tank")
                    print("Spawned: Tank (manual)")
                elif event.key == pygame.K_e:
                    game.spawn_enemy("splitter")
                    print("Spawned: Splitter (manual)")
                elif event.key == pygame.K_r:
                    game.spawn_enemy("ghost")
                    print("Spawned: Ghost (manual - invisible)")
                
                # Abilities
                elif event.key == pygame.K_a:
                    if game.activate_ability("apache"):
                        print("Apache Strike activated!")
                    else:
                        print("Can't activate Apache (cost: $50, cooldown or insufficient funds)")
                elif event.key == pygame.K_b:
                    if game.activate_ability("bomber"):
                        print("Bomber Run activated!")
                    else:
                        print("Can't activate Bomber (cost: $75, cooldown or insufficient funds)")
                
                # Toggle ranges
                elif event.key == pygame.K_t:
                    game.show_tower_ranges = not game.show_tower_ranges
                    print(f"Tower ranges: {'ON' if game.show_tower_ranges else 'OFF'}")
            
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                pos = event.pos
                matrix_x = pos[0] // matrix.pixel_size
                matrix_y = pos[1] // matrix.pixel_size
                if game.handle_click(matrix_x, matrix_y):
                    print(f"Placed {game.selected_tower_type} at ({matrix_x}, {matrix_y})")
                else:
                    print("Cannot place tower there")
        
        # Update and draw
        game.update(dt)
        game.draw()
        
        # Get wave info for display
        wave_info = game.get_wave_info()
        wave_state = wave_info['state']
        
        # Build wave status string
        if wave_state == 'waiting':
            wave_status = "READY (Press SPACE)"
        elif wave_state == 'wave_active':
            wave_status = f"Wave {wave_info['current']}/{wave_info['total']}"
        elif wave_state == 'wave_complete':
            next_wave_time = wave_info['time_until_next']
            if game.wave_manager.auto_start:
                wave_status = f"Next: {next_wave_time:.1f}s"
            else:
                wave_status = "READY (Press SPACE)"
        elif wave_state == 'game_won':
            wave_status = "VICTORY!"
        else:
            wave_status = "---"
        
        # Display game info in window title
        apache_cd = game.ability_manager.get_cooldown_remaining("apache", game.game_time)
        bomber_cd = game.ability_manager.get_cooldown_remaining("bomber", game.game_time)
        
        tower_type_name = game.selected_tower_type if game.selected_tower_type else "None"
        
        banner_status = "✈" if (game.banner_plane and game.banner_plane.active) else ""
        
        game_status = ""
        if game.game_over:
            game_status = "GAME OVER | "
        elif game.game_won:
            game_status = "VICTORY! | "
        
        pygame.display.set_caption(
            f"TD | {game_status}${game.money} | ❤{game.lives} | Score:{game.score} | "
            f"{wave_status} | Enemies:{len(game.enemies)} | "
            f"Tower:{tower_type_name} | Apache:{apache_cd:.1f}s | Bomber:{bomber_cd:.1f}s {banner_status}"
        )
        
        if not matrix.update_display():
            running = False
        
        clock.tick(30)
    
    matrix.close()


if __name__ == "__main__":
    main()