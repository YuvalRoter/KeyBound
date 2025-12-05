#pragma once

class Direction {
    int dirx, diry;
    friend class Point;
public:
    enum {
        UP = 0,
        RIGHT = 1,
        DOWN = 2,
        LEFT = 3,
        STAY = 4,
        NUM_DIRECTIONS = 5
    };

    static const Direction directions[NUM_DIRECTIONS];


    Direction() : dirx(0), diry(0) {}

    Direction(int dir_x, int dir_y)
        : dirx(dir_x), diry(dir_y) {
    }

    Direction operator*(int scalar) const {
        return Direction(dirx * scalar, diry * scalar);
    }
    int getDirX() const { return dirx; }
    int getDirY() const { return diry; }
};