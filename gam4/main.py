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
    map_data = MapLoader.load_json("maps/forest.json")
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    game = Game(matrix, map_data)
    
    # Game loop
    clock = pygame.time.Clock()
    last_spawn_time = 0
    auto_spawn_interval = 2.5
    running = True
    
    while running:
        dt = clock.get_time() / 1000.0
        
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_SPACE:
                    game.spawn_enemy("basic")
                elif event.key == pygame.K_f:
                    game.spawn_enemy("fast")
                elif event.key == pygame.K_t:
                    game.spawn_enemy("tank")
                elif event.key == pygame.K_1:
                    game.selected_tower_type = "dart"
                elif event.key == pygame.K_2:
                    game.selected_tower_type = "tack"
                elif event.key == pygame.K_3:
                    game.selected_tower_type = "bomb"
                elif event.key == pygame.K_a:
                    # Apache Strike ability
                    if game.activate_ability("apache"):
                        print("Apache Strike activated!")
                    else:
                        print("Can't activate Apache (cost: 50, cooldown or insufficient funds)")
                elif event.key == pygame.K_b:
                    # Bomber ability
                    if game.activate_ability("bomber"):
                        print("Bomber Run activated!")
                    else:
                        print("Can't activate Bomber (cost: 75, cooldown or insufficient funds)")
                elif event.key == pygame.K_r:
                    game.show_tower_ranges = not game.show_tower_ranges
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                pos = event.pos
                matrix_x = pos[0] // matrix.pixel_size
                matrix_y = pos[1] // matrix.pixel_size
                game.handle_click(matrix_x, matrix_y)
        
        # Auto-spawn enemies
        if game.game_time - last_spawn_time > auto_spawn_interval and not game.game_over:
            game.spawn_enemy("basic")
            last_spawn_time = game.game_time
        
        # Update and draw
        game.update(dt)
        game.draw()
        
        # Display game info in window title
        apache_cd = game.ability_manager.get_cooldown_remaining("apache", game.game_time)
        bomber_cd = game.ability_manager.get_cooldown_remaining("bomber", game.game_time)
        pygame.display.set_caption(
            f"TD | ${game.money} | Lives:{game.lives} | Score:{game.score} | "
            f"Apache:{apache_cd:.1f}s | Bomber:{bomber_cd:.1f}s"
        )
        
        if not matrix.update_display():
            running = False
        
        clock.tick(30)
    
    matrix.close()


if __name__ == "__main__":
    print("Tower Defense Game")
    print("Towers: 1=Dart | 2=Tack | 3=Bomb")
    print("Enemies: SPACE=Basic | F=Fast | T=Tank")
    print("Abilities: A=Apache Strike ($50) | B=Bomber Run ($75)")
    print("Other: R=Show Ranges | Click=Place Tower | ESC=Quit")
    main()