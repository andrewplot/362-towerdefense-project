"""
Game logic and state management for Tower Defense
"""

from map_render import MapRenderer
from enemy import Enemy
from tower import Tower


class Game:
    """Manages game state and logic"""
    
    def __init__(self, matrix, map_data):
        self.matrix = matrix
        self.map_data = map_data
        self.renderer = MapRenderer(matrix, map_data)
        
        # Game state
        self.enemies = []
        self.towers = []
        self.money = 200  # Starting money
        self.lives = 20  # Player health
        self.score = 0
        self.game_time = 0
        self.game_over = False
        
        # Tower placement state
        self.selected_tower_type = None
        self.show_tower_ranges = False
    
    def spawn_enemy(self, enemy_type):
        """Spawn a new enemy"""
        enemy = Enemy(enemy_type, self.map_data.path, self.game_time)
        self.enemies.append(enemy)
    
    def place_tower(self, tower_type, x, y):
        """
        Attempt to place a tower at given coordinates
        
        Returns:
            bool: True if tower was placed successfully
        """
        # Check if we have enough money
        cost = Tower.get_cost(tower_type)
        if self.money < cost:
            return False
        
        # Find nearest tower slot
        slot = self._find_nearest_tower_slot(x, y, max_distance=5)
        if not slot:
            return False
        
        slot_x, slot_y = slot
        
        # Check if slot is already occupied
        for tower in self.towers:
            if tower.x == slot_x and tower.y == slot_y:
                return False
        
        # Place the tower
        tower = Tower(tower_type, slot_x, slot_y)
        self.towers.append(tower)
        self.money -= cost
        return True
    
    def _find_nearest_tower_slot(self, x, y, max_distance=10):
        """Find the nearest tower slot to given coordinates"""
        nearest_slot = None
        nearest_distance = max_distance + 1
        
        for slot_x, slot_y in self.map_data.tower_slots:
            distance = ((slot_x - x) ** 2 + (slot_y - y) ** 2) ** 0.5
            if distance < nearest_distance:
                nearest_distance = distance
                nearest_slot = (slot_x, slot_y)
        
        return nearest_slot if nearest_distance <= max_distance else None
    
    def update(self, dt):
        """Update game state"""
        if self.game_over:
            return
        
        self.game_time += dt
        
        # Update towers
        for tower in self.towers:
            tower.update(dt, self.enemies)
        
        # Update enemies
        for enemy in self.enemies[:]:
            reached_end = enemy.update(dt)
            
            if reached_end:
                self.enemies.remove(enemy)
                self.lives -= enemy.damage
                if self.lives <= 0:
                    self.lives = 0
                    self.game_over = True
            
            elif not enemy.alive:
                self.enemies.remove(enemy)
                self.money += enemy.reward
                self.score += enemy.reward
    
    def draw(self):
        """Draw everything"""
        self.matrix.clear()
        self.renderer.draw_all()
        
        # Draw tower ranges if enabled
        if self.show_tower_ranges:
            for tower in self.towers:
                tower.draw_range(self.matrix)
        
        # Draw towers and enemies
        for tower in self.towers:
            tower.draw(self.matrix)
        for enemy in self.enemies:
            enemy.draw(self.matrix)
    
    def handle_click(self, matrix_x, matrix_y):
        """Handle click for tower placement"""
        if self.selected_tower_type:
            return self.place_tower(self.selected_tower_type, matrix_x, matrix_y)
        return False