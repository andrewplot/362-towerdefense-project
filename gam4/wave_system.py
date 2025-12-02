"""
Wave System for Tower Defense Game
Manages wave progression, enemy spawning, and inter-wave periods
"""


class Wave:
    """Represents a single wave of enemies"""
    
    def __init__(self, wave_data, wave_number):
        """
        Initialize a wave
        
        Args:
            wave_data: Dictionary containing wave configuration
            wave_number: Wave number (1-indexed)
        """
        self.wave_number = wave_number
        self.enemies = wave_data.get("enemies", [])  # List of enemy types to spawn
        self.spawn_interval = wave_data.get("spawn_interval", 1.0)  # Seconds between spawns
        self.reward_multiplier = wave_data.get("reward_multiplier", 1.0)  # Bonus rewards
        
        # State tracking
        self.spawn_index = 0  # Next enemy to spawn
        self.time_since_last_spawn = 0
        self.completed = False
    
    def get_next_enemy(self):
        """
        Get the next enemy type to spawn
        
        Returns:
            str: Enemy type or None if wave is complete
        """
        if self.spawn_index >= len(self.enemies):
            return None
        
        enemy_type = self.enemies[self.spawn_index]
        self.spawn_index += 1
        
        if self.spawn_index >= len(self.enemies):
            self.completed = True
        
        return enemy_type
    
    def is_complete(self):
        """Check if all enemies have been spawned"""
        return self.completed
    
    def reset_spawn_timer(self):
        """Reset the spawn timer"""
        self.time_since_last_spawn = 0


class WaveManager:
    """Manages wave progression and enemy spawning"""
    
    # Game states
    STATE_WAITING = "waiting"  # Waiting to start first wave
    STATE_WAVE_ACTIVE = "wave_active"  # Wave in progress
    STATE_WAVE_COMPLETE = "wave_complete"  # Wave finished, waiting for next
    STATE_GAME_WON = "game_won"  # All waves completed
    
    def __init__(self, wave_definitions):
        """
        Initialize wave manager
        
        Args:
            wave_definitions: List of wave data dictionaries from map JSON
        """
        self.wave_definitions = wave_definitions
        self.current_wave_index = -1  # -1 = no wave started yet
        self.current_wave = None
        self.state = self.STATE_WAITING
        
        # Inter-wave settings
        self.inter_wave_delay = 5.0  # Seconds between waves
        self.time_until_next_wave = self.inter_wave_delay
        
        # Auto-start settings
        self.auto_start = False  # Auto-start next wave after delay
        self.show_banner = True  # Show banner plane between waves
    
    def start_next_wave(self):
        """
        Start the next wave
        
        Returns:
            Wave object or None if no more waves
        """
        self.current_wave_index += 1
        
        if self.current_wave_index >= len(self.wave_definitions):
            self.state = self.STATE_GAME_WON
            return None
        
        wave_data = self.wave_definitions[self.current_wave_index]
        self.current_wave = Wave(wave_data, self.current_wave_index + 1)
        self.state = self.STATE_WAVE_ACTIVE
        
        return self.current_wave
    
    def update(self, dt, game):
        """
        Update wave state and spawn enemies
        
        Args:
            dt: Delta time in seconds
            game: Game instance for spawning enemies and banner plane
        """
        if self.state == self.STATE_WAITING:
            # Waiting for player to start first wave
            pass
        
        elif self.state == self.STATE_WAVE_ACTIVE:
            # Active wave - spawn enemies
            if not self.current_wave:
                return
            
            self.current_wave.time_since_last_spawn += dt
            
            # Check if it's time to spawn next enemy
            if self.current_wave.time_since_last_spawn >= self.current_wave.spawn_interval:
                enemy_type = self.current_wave.get_next_enemy()
                
                if enemy_type:
                    game.spawn_enemy(enemy_type)
                    self.current_wave.reset_spawn_timer()
            
            # Check if wave spawning is complete
            if self.current_wave.is_complete():
                # Check if all spawned enemies are dead or reached end
                if len(game.enemies) == 0:
                    self._complete_wave(game)
        
        elif self.state == self.STATE_WAVE_COMPLETE:
            # Waiting between waves
            self.time_until_next_wave -= dt
            
            if self.time_until_next_wave <= 0 and self.auto_start:
                self._start_next_wave_with_banner(game)
    
    def _complete_wave(self, game):
        """Handle wave completion"""
        self.state = self.STATE_WAVE_COMPLETE
        self.time_until_next_wave = self.inter_wave_delay
        
        # Award bonus money for completing wave
        wave_bonus = 20 + (self.current_wave_index * 10)
        game.money += wave_bonus
        game.score += wave_bonus
    
    def _start_next_wave_with_banner(self, game):
        """Start next wave and show banner if enabled"""
        wave = self.start_next_wave()
        
        if wave and self.show_banner:
            game.spawn_banner_plane(wave.wave_number)
    
    def manual_start_wave(self, game):
        """
        Manually start the next wave (called by player input)
        
        Returns:
            bool: True if wave started, False if not ready
        """
        if self.state == self.STATE_WAITING:
            self._start_next_wave_with_banner(game)
            return True
        elif self.state == self.STATE_WAVE_COMPLETE:
            self._start_next_wave_with_banner(game)
            return True
        return False
    
    def get_wave_info(self):
        """
        Get current wave information
        
        Returns:
            dict: Wave info (wave number, total waves, state, etc)
        """
        return {
            "current": self.current_wave_index + 1,
            "total": len(self.wave_definitions),
            "state": self.state,
            "time_until_next": max(0, self.time_until_next_wave),
            "enemies_remaining": self.current_wave.spawn_index if self.current_wave else 0
        }
    
    def is_wave_active(self):
        """Check if a wave is currently active"""
        return self.state == self.STATE_WAVE_ACTIVE
    
    def is_game_won(self):
        """Check if all waves are complete"""
        return self.state == self.STATE_GAME_WON