"""
Map Renderer for Tower Defense Game
Draws map data to the LED matrix
"""

class MapRenderer:
    """Draws map data to the LED matrix"""
    # Color definitions
    PATH_COLOR = (100, 100, 100)      # gray path
    TOWER_SLOT_COLOR = (128, 107, 0)  # gold tower slots
    TOWER_SELECTION_COLOR = (255, 215, 0)
    SPAWN_COLOR = (255, 255, 0)       # Yellow spawn
    END_COLOR = (255, 0, 255)         # Magenta end
    
    # Decoration colors
    TREE_COLOR = (0, 100, 0)
    TREETRUNK_COLOR = (100, 50, 0)
    ROCK_COLOR = (75, 75, 75)
    LAKE_COLOR = (0, 0, 150)
    
    def __init__(self, matrix, map_data):
        """
        Initialize the map renderer
        
        Args:
            matrix: LEDMatrixSimulator instance
            map_data: MapData instance to render
        """
        self.matrix = matrix
        self.map_data = map_data
    
    def draw_background(self):
        """Draw the background color"""
        r, g, b = self.map_data.background_color
        self.matrix.fill_rect(0, 0, self.matrix.width, self.matrix.height, r, g, b)
    
    def draw_decorations(self):
        """Draw decorative elements (trees, rocks, etc)"""
        for deco in self.map_data.decorations:
            deco_type = deco.get("type", "tree")
            x = deco.get("x", 0)
            y = deco.get("y", 0)
            
            if deco_type == "tree":
                self.matrix.fill_rect(x - 1, y - 1, 3, 3, *self.TREE_COLOR)
                self.matrix.fill_rect(x, y + 2, 1, 2, *self.TREETRUNK_COLOR)
                self.matrix.fill_rect(x - 1, y + 3, 3, 1, *self.TREETRUNK_COLOR)
            elif deco_type == "rock":
                self.matrix.fill_rect(x - 1, y - 1, 2, 2, *self.ROCK_COLOR)
                self.matrix.fill_rect(x + 1, y, 1, 1, *self.ROCK_COLOR)
            elif deco_type == "lake":
                self.matrix.fill_rect(x-1, y-1, 6, 2, *self.LAKE_COLOR)
                self.matrix.fill_rect(x, y-2, 3, 1, *self.LAKE_COLOR)
                self.matrix.fill_rect(x+1, y+1, 3, 1, *self.LAKE_COLOR)

    
    def draw_path(self):
        """Draw the enemy path (3 pixels wide)"""
        # Draw lines between waypoints
        for i in range(len(self.map_data.path) - 1):
            x1, y1 = self.map_data.path[i]
            x2, y2 = self.map_data.path[i + 1]
            
            # Draw center line
            self.matrix.draw_line(x1, y1, x2, y2, *self.PATH_COLOR)
            
            # Make path 3 pixels wide
            if y1 == y2:  #horizontal path
                self.matrix.draw_line(x1, y1 - 1, x2, y2 - 1, *self.PATH_COLOR)
                self.matrix.draw_line(x1, y1 + 1, x2, y2 + 1, *self.PATH_COLOR)
            else:  # vertical pathp
                self.matrix.draw_line(x1 - 1, y1, x2 - 1, y2, *self.PATH_COLOR)
                self.matrix.draw_line(x1 + 1, y1, x2 + 1, y2, *self.PATH_COLOR)
    
    def draw_tower_slots(self):
        """Draw tower placement locations"""
        for x, y in self.map_data.tower_slots:
            self.matrix.fill_rect(x - 2, y - 2, 4, 4, *self.TOWER_SLOT_COLOR)
    
    def draw_spawn_and_end(self):
        """Draw spawn and end points"""
        sx, sy = self.map_data.spawn_point
        self.matrix.fill_rect(sx - 1, sy - 1, 2, 2, *self.SPAWN_COLOR)
        
        ex, ey = self.map_data.end_point
        self.matrix.fill_rect(ex - 1, ey - 1, 2, 2, *self.END_COLOR)
    
    def draw_all(self):
        """Draw the complete map (call this every frame)"""
        self.draw_background()
        self.draw_decorations()
        self.draw_path()
        self.draw_tower_slots()
        # self.draw_spawn_and_end()


# Test code
if __name__ == "__main__":
    from led_matrix import LEDMatrixSimulator
    from map_data import MapLoader
    import pygame
    
    print("Testing Map Renderer...")
    
    # Load a map
    try:
        map_data = MapLoader.load_json("maps/map1.json")
    except FileNotFoundError:
        print("Error: Run map_data.py first to create sample maps!")
        exit(1)
    
    print(f"Loaded: {map_data.name}")
    
    # Create matrix and renderer
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    renderer = MapRenderer(matrix, map_data)
    
    # Draw the map
    matrix.clear()
    renderer.draw_all()
    matrix.update_display()
    
    print("Displaying map. Close window to exit...")
    
    # Keep window open
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        matrix.tick(10)
    
    matrix.close()
    print("Done!")