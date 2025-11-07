class Enemy:
    
    # Enemy type definitions
    ENEMY_TYPES = {
        "basic": {
            "health": 1,
            "speed": 5,
            "color": (255, 50, 50),
            "reward": 1,
            "damage": 1,
            "size": 1
        }
    }
    
    def __init__(self, enemy_type, path, spawn_time):
        """
        Initialize an enemy
        
        Args:
            enemy_type: Type of enemy ("basic")
            path: List of (x, y) waypoints to follow
            spawn_time: Time when enemy was spawned
        """
        self.enemy_type = enemy_type
        self.path = path
        self.spawn_time = spawn_time
        
        # Load stats from enemy type
        stats = self.ENEMY_TYPES[enemy_type]
        self.max_health = stats["health"]
        self.health = self.max_health
        self.speed = stats["speed"]
        self.color = stats["color"]
        self.reward = stats["reward"]
        self.damage = stats["damage"]
        self.size = stats["size"]
        
        # Position tracking
        self.x, self.y = path[0]  # Start at first waypoint
        self.path_index = 0  # Current waypoint we're moving toward
        self.path_progress = 0.0  # Total distance traveled along path
        
        self.alive = True
    
    def update(self, dt):
        """
        Move enemy along the path
        
        Args:
            dt: Delta time (time since last update)
            
        Returns:
            bool: True if reached end of path, False otherwise
        """
        if self.path_index >= len(self.path) - 1:
            # Reached the end
            return True
        
        # Get current target waypoint
        target_x, target_y = self.path[self.path_index + 1]
        
        # Calculate direction to target
        dx = target_x - self.x
        dy = target_y - self.y
        distance = (dx * dx + dy * dy) ** 0.5
        
        if distance == 0:
            # Already at waypoint, move to next
            self.path_index += 1
            return False
        
        # Calculate movement distance this frame
        move_distance = self.speed * dt
        
        if distance <= move_distance:
            # Reached waypoint, move to next
            self.x = target_x
            self.y = target_y
            self.path_progress += distance
            self.path_index += 1
        else:
            # Move toward waypoint
            # Normalize direction and scale by move_distance
            self.x += (dx / distance) * move_distance
            self.y += (dy / distance) * move_distance
            self.path_progress += move_distance
        
        return False  # Not at end yet
    
    def take_damage(self, damage):
        """
        Apply damage to enemy
        
        Args:
            damage: Amount of damage to apply
        """
        self.health -= damage
        if self.health <= 0:
            self.alive = False
    
    def draw(self, matrix):
        """Draw the enemy on the LED matrix"""
        # Draw enemy as a colored circle (using filled rect for simplicity)
        half_size = self.size // 2
        x = int(self.x)
        y = int(self.y)
        matrix.fill_rect(
            x - half_size, 
            y - half_size, 
            self.size, 
            self.size, 
            *self.color
        )