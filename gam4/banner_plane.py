"""
Banner Plane for Tower Defense Game
Displays wave announcements between rounds
"""

import math


class BannerPlane:
    """
    Red propeller plane that flies across screen with a waving banner
    displaying "WAVE X" between rounds
    """
    
    def __init__(self, start_x, start_y, game_time, wave_number):
        """
        Initialize the Banner Plane
        
        Args:
            start_x, start_y: Starting position (usually off-screen left)
            game_time: Current game time
            wave_number: Wave number to display on banner
        """
        self.x = start_x
        self.y = start_y
        self.spawn_time = game_time
        self.game_time = game_time
        self.wave_number = wave_number
        
        # Movement (flies left to right across screen)
        self.speed = 15  # Pixels per second
        self.target_x = 70  # Fly off screen to the right
        
        # Visual
        self.plane_color = (200, 20, 20)  # Bright red
        self.propeller_color = (150, 150, 150)  # Gray
        self.banner_color = (220, 220, 180)  # Off-white with slight yellow tint
        self.text_color = (200, 0, 0)  # Dark red text
        
        # Banner properties
        self.banner_length = 35  # Length of banner in pixels
        self.banner_height = 13  # Height of banner (even taller)
        self.wave_amplitude = 2  # How much the banner waves (pixels)
        self.wave_frequency = 3  # Wave speed
        
        # State
        self.active = True
    
    def update(self, dt):
        """
        Update plane movement
        
        Args:
            dt: Delta time in seconds
        """
        # Update game time for animation
        self.game_time += dt
        
        # Move right across screen
        self.x += self.speed * dt
        
        # Check if off-screen
        if self.x > self.target_x:
            self.active = False
    
    def draw(self, matrix):
        """Draw the banner plane on the LED matrix"""
        x = int(self.x)
        y = int(self.y)
        
        # Draw banner FIRST (so plane overlays it)
        self._draw_banner(matrix, x, y)
        
        # Draw plane body (side view, facing right)
        plane_body_color = self.plane_color
        
        # Fuselage (main body - horizontal)
        matrix.fill_rect(x - 2, y - 1, 5, 2, *plane_body_color)
        
        # Nose cone (pointed front)
        matrix.set_pixel(x + 3, y, *plane_body_color)
        matrix.set_pixel(x + 4, y, 220, 40, 40)  # Brighter red nose tip
        
        # Cockpit (darker section)
        cockpit_color = (120, 10, 10)  # Dark red
        matrix.fill_rect(x + 1, y - 1, 2, 2, *cockpit_color)
        
        # Wings (extending above and below, symmetrical)
        wing_color = (180, 20, 20)  # Slightly darker red
        # Top wing (2 pixels wide, 3 pixels from center)
        matrix.fill_rect(x, y - 3, 2, 1, *wing_color)
        matrix.fill_rect(x, y - 2, 2, 1, *wing_color)
        # Bottom wing (2 pixels wide, 2-3 pixels from center)
        matrix.fill_rect(x, y + 2, 2, 1, *wing_color)
        matrix.fill_rect(x, y + 3, 2, 1, *wing_color)
        
        # Tail fin (back, centered)
        matrix.fill_rect(x - 3, y - 1, 1, 2, *wing_color)
        matrix.set_pixel(x - 4, y, *wing_color)
        
        # Propeller (animated 4-phase spin)
        prop_phase = int((self.game_time * 20) % 4)
        prop_color = self.propeller_color
        
        if prop_phase == 0:
            # Vertical
            matrix.set_pixel(x + 5, y - 1, *prop_color)
            matrix.set_pixel(x + 5, y + 1, *prop_color)
        elif prop_phase == 1:
            # Diagonal /
            matrix.set_pixel(x + 5, y - 1, *prop_color)
            matrix.set_pixel(x + 5, y + 1, *prop_color)
        elif prop_phase == 2:
            # Horizontal
            matrix.set_pixel(x + 4, y, *prop_color)
            matrix.set_pixel(x + 6, y, *prop_color)
        else:
            # Diagonal \
            matrix.set_pixel(x + 5, y - 1, *prop_color)
            matrix.set_pixel(x + 5, y + 1, *prop_color)
        
        # Propeller hub
        matrix.set_pixel(x + 5, y, 100, 100, 100)
    
    def _draw_banner(self, matrix, plane_x, plane_y):
        """Draw the waving banner trailing behind the plane"""
        # Banner attaches to back of plane
        banner_start_x = plane_x - 3
        banner_y = plane_y
        
        # Draw banner segments with wave effect
        for i in range(self.banner_length):
            # Calculate wave offset using sine wave
            wave_phase = self.game_time * self.wave_frequency + i * 0.3
            wave_offset = int(math.sin(wave_phase) * self.wave_amplitude)
            
            segment_x = banner_start_x - i
            segment_y = banner_y + wave_offset
            
            # Don't draw if off-screen
            if segment_x < -5:
                continue
            
            # Draw banner vertical strip (multi-pixel height for visibility)
            for dy in range(-self.banner_height // 2, self.banner_height // 2 + 1):
                pixel_y = segment_y + dy
                
                # White banner background
                matrix.set_pixel(segment_x, pixel_y, *self.banner_color)
                
                # Draw text "WAVE X" on banner
                self._draw_banner_text(matrix, segment_x, pixel_y, i)
    
    def _draw_banner_text(self, matrix, x, y, position):
        """Draw text on the banner"""
        # Text starts after a few pixels of blank banner
        text_start = 5
        text = f"WAVE {self.wave_number}"
        
        # Wider 5x5 pixel font for better readability
        # Position in banner determines which character to draw
        char_width = 6  # 5 pixels + 1 space
        
        if position < text_start:
            return  # Blank leading edge
        
        text_pos = position - text_start
        char_index = text_pos // char_width
        pixel_in_char = text_pos % char_width
        
        if char_index >= len(text) or pixel_in_char >= 5:
            return  # Space between chars or past end
        
        char = text[char_index]
        
        # Get the pixel pattern for this character
        pattern = self._get_char_pattern(char, pixel_in_char)
        
        # Draw the vertical line of pixels for this position
        for bit_y, bit in enumerate(pattern):
            if bit:
                draw_y = y - 3 + bit_y  # Center text vertically (adjusted for taller banner)
                matrix.set_pixel(x, draw_y, *self.text_color)
    
    def _get_char_pattern(self, char, col):
        """
        Get the pixel pattern for a character column
        Returns a list of 5 bools representing pixels
        """
        # 5x5 bitmap font (each char is 5 columns wide, 5 pixels tall)
        # Returns the column pattern for the given column index (0-4)
        
        patterns = {
            'W': [
                [1, 1, 1, 1, 1],  # Col 0: left vertical
                [0, 0, 0, 1, 1],  # Col 1
                [0, 0, 1, 1, 0],  # Col 2: middle peak
                [0, 0, 0, 1, 1],  # Col 3
                [1, 1, 1, 1, 1],  # Col 4: right vertical
            ],
            'A': [
                [0, 1, 1, 1, 1],  # Col 0: left side
                [1, 0, 1, 0, 0],  # Col 1: gap + crossbar
                [1, 0, 0, 0, 0],  # Col 2: top only
                [1, 0, 1, 0, 0],  # Col 3: gap + crossbar
                [0, 1, 1, 1, 1],  # Col 4: right side
            ],
            'V': [
                [1, 1, 1, 0, 0],  # Col 0
                [0, 0, 0, 1, 1],  # Col 1
                [0, 0, 0, 0, 1],  # Col 2: bottom point
                [0, 0, 0, 1, 1],  # Col 3
                [1, 1, 1, 0, 0],  # Col 4
            ],
            'E': [
                [1, 1, 1, 1, 1],  # Col 0: vertical
                [1, 0, 1, 0, 1],  # Col 1: crossbars only
                [1, 0, 1, 0, 1],  # Col 2: crossbars only
                [1, 0, 1, 0, 1],  # Col 3: crossbars only
                [1, 0, 0, 0, 1],  # Col 4: top and bottom only
            ],
            ' ': [
                [0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0],
            ],
            '1': [
                [0, 0, 0, 0, 0],  # Col 0
                [0, 1, 0, 0, 0],  # Col 1: left stub
                [1, 1, 1, 1, 1],  # Col 2: main vertical
                [0, 0, 0, 0, 0],  # Col 3
                [0, 0, 0, 0, 0],  # Col 4
            ],
            '2': [
                [1, 0, 0, 1, 1],  # Col 0
                [1, 0, 0, 1, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: middle diagonal
                [1, 1, 0, 0, 1],  # Col 3
                [1, 1, 0, 0, 1],  # Col 4
            ],
            '3': [
                [1, 0, 0, 0, 1],  # Col 0
                [1, 0, 0, 0, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: middle bar
                [0, 1, 0, 1, 0],  # Col 3: curves
                [0, 1, 1, 1, 0],  # Col 4: curves
            ],
            '4': [
                [1, 1, 1, 0, 0],  # Col 0: top horizontal
                [0, 0, 1, 0, 0],  # Col 1
                [0, 0, 1, 0, 0],  # Col 2
                [1, 1, 1, 1, 1],  # Col 3: vertical + crossbar
                [0, 0, 1, 1, 1],  # Col 4: bottom
            ],
            '5': [
                [1, 1, 0, 0, 1],  # Col 0
                [1, 1, 0, 0, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: middle
                [1, 0, 0, 1, 1],  # Col 3
                [1, 0, 0, 1, 0],  # Col 4
            ],
            '6': [
                [0, 1, 1, 1, 1],  # Col 0
                [1, 1, 1, 1, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: crossbars
                [1, 0, 0, 1, 1],  # Col 3
                [0, 0, 0, 1, 0],  # Col 4
            ],
            '7': [
                [1, 0, 0, 0, 0],  # Col 0: top
                [1, 0, 0, 0, 0],  # Col 1
                [1, 0, 0, 1, 1],  # Col 2
                [1, 1, 1, 1, 0],  # Col 3: diagonal
                [1, 1, 0, 0, 0],  # Col 4
            ],
            '8': [
                [0, 1, 1, 1, 0],  # Col 0: rounded
                [1, 1, 1, 1, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: crossbars
                [1, 1, 1, 1, 1],  # Col 3
                [0, 1, 1, 1, 0],  # Col 4: rounded
            ],
            '9': [
                [0, 1, 0, 0, 1],  # Col 0
                [1, 1, 0, 0, 1],  # Col 1
                [1, 0, 1, 0, 1],  # Col 2: crossbar
                [1, 1, 1, 1, 1],  # Col 3
                [1, 1, 1, 1, 0],  # Col 4
            ],
            '0': [
                [0, 1, 1, 1, 0],  # Col 0: rounded
                [1, 1, 1, 1, 1],  # Col 1
                [1, 0, 0, 0, 1],  # Col 2: hollow
                [1, 1, 1, 1, 1],  # Col 3
                [0, 1, 1, 1, 0],  # Col 4: rounded
            ],
        }
        
        if char in patterns and col < 5:
            return patterns[char][col]
        
        return [0, 0, 0, 0, 0]  # Unknown char = blank


# Test code
if __name__ == "__main__":
    from led_matrix import LEDMatrixSimulator
    import pygame
    
    print("Testing Banner Plane...")
    
    # Create matrix
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    
    # Create banner plane
    banner = BannerPlane(-45, 16, 0, 5)  # Wave 5
    
    print("Banner plane flying across screen...")
    print("Close window to exit...")
    
    # Animation loop
    clock = pygame.time.Clock()
    running = True
    game_time = 0
    
    while running and banner.active:
        dt = clock.get_time() / 1000.0
        game_time += dt
        
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        
        # Update and draw
        banner.update(dt)
        
        matrix.clear()
        # Draw simple background
        matrix.fill_rect(0, 0, 64, 32, 0, 50, 0)
        banner.draw(matrix)
        
        if not matrix.update_display():
            running = False
        
        clock.tick(30)
    
    print("Banner plane finished!")
    matrix.close()