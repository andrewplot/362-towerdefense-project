PSEUDOCODE Types.h:

// Basic geometric types
STRUCT Point:
    int16_t x
    int16_t y
    
    CONSTRUCTOR Point(x, y)
    
    FUNCTION distance_to(other_point) -> float:
        dx = other_point.x - x
        dy = other_point.y - y
        RETURN sqrt(dx*dx + dy*dy)

// Color representation
STRUCT Color:
    uint8_t r, g, b
    
    CONSTRUCTOR Color(r, g, b)
    STATIC CONSTANT predefined_colors (RED, GREEN, BLUE, etc.)

// Fixed-point arithmetic for precise movement
TYPEDEF int32_t fixed_point_t  // 16.16 fixed point

FUNCTION float_to_fixed(float_value) -> fixed_point_t:
    RETURN (int32_t)(float_value * 65536.0f)

FUNCTION fixed_to_float(fixed_value) -> float:
    RETURN fixed_value / 65536.0f

// Rectangle for collision detection
STRUCT Rect:
    int16_t x, y, width, height
    
    FUNCTION contains(point) -> bool
    FUNCTION intersects(other_rect) -> bool