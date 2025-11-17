#pragma once

#include <utility>
#include <cmath>

#include "Direction.h"

class Point {
	int x = 0, y = 0;
	Direction dir = Direction::directions[Direction::RIGHT];
	char ch = '*';
public:
	Point() {}
	Point(int x1, int y1, const Direction& the_dir, char c) {
		x = x1;
		y = y1;
		dir = the_dir;
		ch = c;
	}
	void draw() {
		draw(ch);
	}
	void draw(char c);
	void move();
	void changeDir(const Direction& new_dir) {
		dir = new_dir;
	}
	int getX() const {
		return x;
	}
	int getY() const {
		return y;
	}
	char getChar() const {
		return ch;
	}
};

