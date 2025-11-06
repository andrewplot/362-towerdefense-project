#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

#define BLACK ((Color) {0, 0, 0})
#define WHITE ((Color) {255, 255, 255})

#endif