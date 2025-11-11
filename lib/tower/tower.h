#ifndef TOWER_H
#define TOWER_H

enum TowerType {
    dart,
    ninja,
    bomb,
    sniper,
    blank,
};

typedef struct {
    int type;
    int x_pos;
    int y_pos;
} Tower;

#endif //TOWER_H