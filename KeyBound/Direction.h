#pragma once

class Direction {
	int dirx, diry;
	friend class Point;
public:
	enum { UP, RIGHT, DOWN, LEFT, STAY, NUM_DIRECTIONS };
	static const Direction directions[NUM_DIRECTIONS];
	Direction(int dir_x, int dir_y)
		: dirx(dir_x), diry(dir_y) {
	}
};

