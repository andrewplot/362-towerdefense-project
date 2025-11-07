# main.py - Updated version

import pygame
from led_matrix import LEDMatrixSimulator
from map_data import MapLoader, create_sample_maps
from map_render import MapRenderer
from enemy import Enemy
import os


class Game:
    def __init__(self, matrix, map_data):
        self.matrix = matrix
        self.map_data = map_data
        self.renderer = MapRenderer(matrix, map_data)
        
        # Game state
        self.enemies = []
        self.money = 100  # Starting money
        self.lives = 20  # Player health
        self.score = 0
        self.game_time = 0
        self.game_over = False
    
    def spawn_enemy(self, enemy_type):
        """Spawn a new enemy"""
        enemy = Enemy(enemy_type, self.map_data.path, self.game_time)
        self.enemies.append(enemy)
        print(f"Spawned {enemy_type} bloon")
    
    def update(self, dt):
        """Update game state"""
        if self.game_over:
            return
        
        self.game_time += dt
        
        # Update all enemies
        for enemy in self.enemies[:]:  # Create copy to allow removal
            reached_end = enemy.update(dt)
            
            if reached_end:
                self.enemies.remove(enemy)
                self.lives -= enemy.damage
                print(f"{enemy.enemy_type} bloon reached end. -{enemy.damage} life | Lives remaining: {self.lives}")
                
                # Check for game over
                if self.lives <= 0:
                    self.lives = 0
                    self.game_over = True
                    print("=" * 50)
                    print("GAME OVER!")
                    print(f"Final Score: {self.score}")
                    print("=" * 50)
            
            elif not enemy.alive:
                # Enemy was killed (will be implemented with towers later)
                self.enemies.remove(enemy)
                self.money += enemy.reward
                self.score += enemy.reward
                print(f"{enemy.enemy_type} bloon popped. +${enemy.reward} | Money: ${self.money}")
    
    def draw(self):
        """Draw everything"""
        self.matrix.clear()
        self.renderer.draw_all()
        
        # Draw enemies
        for enemy in self.enemies:
            enemy.draw(self.matrix)
        
        # Draw lives indicator in top-left corner
        # self.draw_lives_indicator()
    
    # def draw_lives_indicator(self):
    #     """Draw player lives as hearts/blocks in corner"""
    #     # Draw up to 10 hearts (or show number if > 10)
    #     if self.lives <= 10:
    #         for i in range(self.lives):
    #             x = 2 + (i * 3)
    #             y = 2
    #             self.matrix.fill_rect(x, y, 2, 2, 255, 0, 0)  # Red hearts
    #     else:
    #         # Just draw a few hearts and rely on LCD for exact count
    #         for i in range(5):
    #             x = 2 + (i * 3)
    #             y = 2
    #             self.matrix.fill_rect(x, y, 2, 2, 255, 0, 0)


def main():
    """Main game loop"""
    
    # Create maps directory and sample maps if needed
    if not os.path.exists("maps"):
        print("Creating sample maps...")
        os.makedirs("maps", exist_ok=True)
        maps = create_sample_maps()
        for i, map_data in enumerate(maps, 1):
            MapLoader.save_json(f"maps/map{i}.json", map_data)
        print(f"Created {len(maps)} sample maps in ./maps/")
    
    # Load the first map
    map_data = MapLoader.load_json("maps/forest.json")
    print(f"Loaded map: {map_data.name}")
    print(f"Path has {len(map_data.path)} waypoints")
    print()
    
    # Initialize display
    print("Starting display...")
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    
    # Create game
    game = Game(matrix, map_data)
    
    print("=" * 50)
    print("Controls:")
    print("  SPACE - Spawn a basic bloon")
    print("  ESC   - Quit")
    print("=" * 50)
    print(f"Lives: {game.lives} | Money: ${game.money}")
    print()
    
    # Main game loop
    running = True
    clock = pygame.time.Clock()
    last_spawn_time = 0
    auto_spawn_interval = 3.0  # Seconds between auto-spawns
    
    while running:
        # Calculate delta time
        dt = clock.get_time() / 1000.0  # Convert to seconds
        
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_SPACE:
                    # Spawn enemy on spacebar for testing
                    game.spawn_enemy("basic")
        
        # Auto-spawn enemies every few seconds for testing
        if game.game_time - last_spawn_time > auto_spawn_interval and not game.game_over:
            game.spawn_enemy("basic")
            last_spawn_time = game.game_time
        
        # Update game logic
        game.update(dt)
        
        # Draw everything
        game.draw()
        
        # Update display
        if not matrix.update_display():
            running = False
        
        # Control frame rate
        clock.tick(30)  # 30 FPS
    
    # Cleanup
    matrix.close()
    print()
    print("Game closed. Goodbye!")


if __name__ == "__main__":
    print("=" * 50)
    print("Tower Defense Game - Python Prototype")
    print("=" * 50)
    print()
    main()