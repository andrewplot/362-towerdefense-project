"""
Tower Defense Game - Main Entry Point
Run this file to start the game
"""

import pygame
from led_matrix import LEDMatrixSimulator
from map_data import MapLoader, create_sample_maps
from map_render import MapRenderer
import os


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
    print(map_data)
    print()
    
    # Initialize display
    print("Starting display...")
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    renderer = MapRenderer(matrix, map_data)
    
    # Main game loop
    running = True
    clock = pygame.time.Clock()
    
    while running:
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
        
        # Update game logic (TODO: add game logic here)
        
        # Draw everything
        matrix.clear()
        renderer.draw_all()
        
        # Update display
        if not matrix.update_display():
            running = False
        
        # Control frame rate
        clock.tick(30)  # 30 FPS
    
    # Cleanup
    matrix.close()
    print("Game closed. Goodbye!")


if __name__ == "__main__":
    print("=" * 50)
    print("Tower Defense Game - Python Prototype")
    print("=" * 50)
    print()
    main()