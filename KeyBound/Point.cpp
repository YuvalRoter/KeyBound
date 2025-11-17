#include <iostream>
#include "Point.h"
#include "utils.h"
#include "Screen.h"

void Point::draw(char c) {
	gotoxy(x, y);
	std::cout << c;
}

void Point::move() {
	if (x + dir.dirx < 0 || x + dir.dirx > Screen::MAX_X) {
		dir.dirx = -dir.dirx;
	}
	if (y + dir.diry < 0 || y + dir.diry > Screen::MAX_Y) {
		dir.diry = -dir.diry;
	}
	x += dir.dirx;
	y += dir.diry;
}
