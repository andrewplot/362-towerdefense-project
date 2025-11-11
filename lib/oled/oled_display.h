#include <stdbool.h>
#include <stdint.h>
#include "hardware/spi.h"

void init_oled();
void print_message(const char *lines[2]);
void demo_heart();