// rpi imports
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// lib imports
#include "../lib/matrix/matrix.c"

int main() {
    stdio_init_all();
    init_matrix();
    render();
    
    return 0;
}

/*  NOTES:

Row select; DCBA (meaning D = 8, C = 4, etc.)
    - This means 1011 = Row 11, 1001 = Row 9, etc 



*/