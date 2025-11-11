import math


class Projectile:
    """Represents a projectile fired by a tower"""
    
    def __init__(self, x, y, target, damage, speed, color):
        """
        Initialize a projectile
        
        Args:
            x, y: Starting position
            target: Enemy object to track
            damage: Damage to deal on hit
            speed: Projectile speed (pixels per second)
            color: RGB tuple for projectile color
        """
        self.x = x
        self.y = y
        self.target = target
        self.damage = damage
        self.speed = speed
        self.color = color
        self.active = True
    
    def update(self, dt):
        """
        Move projectile toward target
        
        Args:
            dt: Delta time in seconds
            
        Returns:
            bool: True if hit target or target died, False if still traveling
        """
        if not self.target.alive:
            self.active = False
            return True
        
        # Calculate direction to target
        dx = self.target.x - self.x
        dy = self.target.y - self.y
        distance = math.sqrt(dx * dx + dy * dy)
        
        if distance == 0:
            # Already at target
            self.target.take_damage(self.damage)
            self.active = False
            return True
        
        # Calculate movement distance this frame
        move_distance = self.speed * dt
        
        if distance <= move_distance:
            # Hit target
            self.target.take_damage(self.damage)
            self.active = False
            return True
        else:
            # Move toward target
            self.x += (dx / distance) * move_distance
            self.y += (dy / distance) * move_distance
            return False
    
    def draw(self, matrix):
        """Draw the projectile on the LED matrix"""
        x = int(self.x)
        y = int(self.y)
        matrix.set_pixel(x, y, *self.color)


class Tower:
    """Base tower class for tower defense game"""
    
    # Tower type definitions
    TOWER_TYPES = {
        "dart": {
            "name": "Dart Monkey",
            "cost": 30,
            "damage": 1,
            "range": 15,
            "fire_rate": 0.8,  # Seconds between shots
            "projectile_speed": 50,  # Pixels per second
            "color": (100, 50, 0),  # Brown
            "projectile_color": (150, 150, 150),  # Gray dart
            "size": 3,
            "description": "Basic tower with decent range"
        },
        "tack": {
            "name": "Tack Shooter",
            "cost": 45,
            "damage": 1,
            "range": 8,
            "fire_rate": 0.5,  # Faster fire rate
            "projectile_speed": 40,
            "color": (200, 0, 0),  # Red
            "projectile_color": (255, 100, 0),  # Orange tack
            "size": 3,
            "shots_per_attack": 4,  # Fires 4 projectiles
            "description": "Short range, fires multiple projectiles"
        },
        "bomb": {
            "name": "Bomb Tower",
            "cost": 65,
            "damage": 3,
            "range": 12,
            "fire_rate": 1.5,  # Slower fire rate
            "projectile_speed": 35,
            "color": (50, 50, 50),  # Dark gray
            "projectile_color": (255, 200, 0),  # Yellow bomb
            "size": 3,
            "splash_radius": 3,  # Damages nearby enemies
            "description": "Explosive damage to multiple enemies"
        }
    }
    
    def __init__(self, tower_type, x, y):
        """
        Initialize a tower
        
        Args:
            tower_type: Type of tower ("dart", "tack", "bomb")
            x, y: Position on the map
        """
        self.tower_type = tower_type
        self.x = x
        self.y = y
        
        # Load stats from tower type
        stats = self.TOWER_TYPES[tower_type]
        self.name = stats["name"]
        self.damage = stats["damage"]
        self.range = stats["range"]
        self.fire_rate = stats["fire_rate"]
        self.projectile_speed = stats["projectile_speed"]
        self.color = stats["color"]
        self.projectile_color = stats["projectile_color"]
        self.size = stats["size"]
        
        # Optional stats
        self.shots_per_attack = stats.get("shots_per_attack", 1)
        self.splash_radius = stats.get("splash_radius", 0)
        
        # State tracking
        self.time_since_last_shot = 0
        self.target = None
        self.projectiles = []
    
    def update(self, dt, enemies):
        """
        Update tower logic (targeting and shooting)
        
        Args:
            dt: Delta time in seconds
            enemies: List of Enemy objects
        """
        self.time_since_last_shot += dt
        
        # Update existing projectiles
        for projectile in self.projectiles[:]:
            if projectile.update(dt):
                self.projectiles.remove(projectile)
        
        # Check if we can shoot
        if self.time_since_last_shot >= self.fire_rate:
            target = self.find_target(enemies)
            if target:
                self.shoot(target)
                self.time_since_last_shot = 0
    
    def find_target(self, enemies):
        """
        Find the best enemy to target
        Uses "first" targeting - enemy furthest along the path
        
        Args:
            enemies: List of Enemy objects
            
        Returns:
            Enemy object or None
        """
        best_target = None
        best_progress = -1
        
        for enemy in enemies:
            if not enemy.alive:
                continue
            
            # Check if enemy is in range
            distance = self.distance_to(enemy)
            if distance <= self.range:
                # Prioritize enemies furthest along the path
                if enemy.path_progress > best_progress:
                    best_progress = enemy.path_progress
                    best_target = enemy
        
        return best_target
    
    def distance_to(self, enemy):
        """Calculate distance to an enemy"""
        dx = enemy.x - self.x
        dy = enemy.y - self.y
        return math.sqrt(dx * dx + dy * dy)
    
    def shoot(self, target):
        """
        Fire projectile(s) at target
        
        Args:
            target: Enemy object to shoot at
        """
        if self.shots_per_attack == 1:
            # Single shot
            projectile = Projectile(
                self.x, self.y,
                target,
                self.damage,
                self.projectile_speed,
                self.projectile_color
            )
            self.projectiles.append(projectile)
        else:
            # Multiple shots (tack shooter style)
            # Distribute projectiles in different directions
            for i in range(self.shots_per_attack):
                # Find enemy in each quadrant if possible, otherwise use main target
                quadrant_target = self.find_quadrant_target(i, target.x, target.y)
                projectile = Projectile(
                    self.x, self.y,
                    quadrant_target if quadrant_target else target,
                    self.damage,
                    self.projectile_speed,
                    self.projectile_color
                )
                self.projectiles.append(projectile)
    
    def find_quadrant_target(self, quadrant, default_x, default_y):
        """Helper for multi-shot towers - can be enhanced later"""
        return None  # For now, all shots go to main target
    
    def draw(self, matrix):
        """Draw the tower on the LED matrix"""
        # Draw tower as a colored square
        half_size = self.size // 2
        matrix.fill_rect(
            int(self.x) - half_size,
            int(self.y) - half_size,
            self.size,
            self.size,
            *self.color
        )
        
        # Draw projectiles
        for projectile in self.projectiles:
            projectile.draw(matrix)
    
    def draw_range(self, matrix):
        """
        Draw tower range indicator (for UI/debugging)
        This draws a circle outline showing the tower's range
        """
        range_color = (100, 100, 100)  # Gray
        
        # Draw circle using midpoint circle algorithm
        radius = int(self.range)
        cx = int(self.x)
        cy = int(self.y)
        
        x = radius
        y = 0
        err = 0
        
        while x >= y:
            # Draw 8 symmetric points
            points = [
                (cx + x, cy + y), (cx + y, cy + x),
                (cx - y, cy + x), (cx - x, cy + y),
                (cx - x, cy - y), (cx - y, cy - x),
                (cx + y, cy - x), (cx + x, cy - y)
            ]
            
            for px, py in points:
                if 0 <= px < matrix.width and 0 <= py < matrix.height:
                    matrix.set_pixel(px, py, *range_color)
            
            if err <= 0:
                y += 1
                err += 2 * y + 1
            
            if err > 0:
                x -= 1
                err -= 2 * x + 1
    
    @classmethod
    def get_cost(cls, tower_type):
        """Get the cost of a tower type"""
        return cls.TOWER_TYPES[tower_type]["cost"]
    
    @classmethod
    def get_info(cls, tower_type):
        """Get information about a tower type"""
        return cls.TOWER_TYPES[tower_type]