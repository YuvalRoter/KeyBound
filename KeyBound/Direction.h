#pragma once

class Direction {
	int dirx, diry;
	friend class Point;
public:
		enum {
		UP = 1,
		DOWN = -1,
		RIGHT = 2,
		LEFT = -2,
		STAY = 0,
		NUM_DIRECTIONS = 5
	
	};
	static const Direction directions[NUM_DIRECTIONS];
	Direction(int dir_x, int dir_y)
		: dirx(dir_x), diry(dir_y) {
	}
	
	Direction operator*(int scalar) const {
		return Direction(dirx * scalar, diry * scalar);
	}
};

