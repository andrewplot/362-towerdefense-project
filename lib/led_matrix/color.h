#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

#define BLACK   ((Color) {  0,   0,   0})
#define WHITE   ((Color) {255, 255, 255})

#define GRASS       ((Color) { 48, 156,  48})
#define GRASS_DARK  ((Color) { 64, 128,  64})
#define PATH        ((Color) {255, 193,   7})
#define TREE_BROWN  ((Color) { 87,  62,  8})
#define TREE_GREEN  ((Color) { 75,  99,  42})

#define BLOON_RED     ((Color) {222,  36,  36})
#define BLOON_BLUE    ((Color) {  0,   0,   0})
#define BLOON_GREEN   ((Color) {  0,   0,   0})
#define BLOON_YELLOW  ((Color) {  0,   0,   0})
#define BLOON_PINK    ((Color) {  0,   0,   0})
#define BLOON_WHITE   ((Color) {  0,   0,   0})
#define BLOON_METAL   ((Color) {  0,   0,   0})

#define DART_RED    ((Color) {222,  8,  8})
#define DART_BROWN  ((Color) { 87,  62,  37})
#define DART_LIGHT  ((Color) {242, 174,  97})

#define NINJA_RED   ((Color) {222,  4,  4})
#define NINJA_WHITE ((Color) {241, 242, 191})

#define BOMB_BLACK  ((Color) {  0,   0,   0})
#define BOMB_BROWN  ((Color) {242, 174,  97})

#define SNIPER_BROWN        ((Color) { 87,  62,  37})
#define SNIPER_LIGHT_BROWN  ((Color) {242, 174,  97})
#define SNIPER_DARK_GREEN   ((Color) { 75,  99,  42})
#define SNIPER_LIGHT_GREEN  ((Color) {145, 145,  63})

#define MONKEY_RED   ((Color) {222,   0,   0})
#define MONKEY_BROWN ((Color) { 87,  62,  37})
#define MONKEY_LIGHT ((Color) {242, 174,  97})

#endif