#ifndef TOWER_HH
#define TOWER_HH

enum TowerType {
    dart,
    ninja,
    bomb,
    sniper,
    blank,

    TOWER_TYPE_COUNT
};


class Tower {
    public:
        TowerType type;
        int x_pos;
        int y_pos;

};

#endif //TOWER_HH