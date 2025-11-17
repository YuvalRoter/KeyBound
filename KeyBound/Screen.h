#pragma once

#include "Point.h"
#include <string>

class Screen {
public:
	static constexpr int MAX_X = 79;
	static constexpr int MAX_Y = 24;
private:

	char screen[MAX_Y + 1][MAX_X + 2];

	char charAt(const Point& p) const {
		return screen[p.getY()][p.getX()];
	}
	

public:
	bool loadFromFile(const std::string& filename);

	bool isWall(const Point& p) const {
		return charAt(p) == 'W';
	}
	bool isWonChar(const Point& p) const {
		return charAt(p) == '%';
	}
	void draw() const;

	char getCharAt(int y, int x) const {
		return screen[y][x];
	}
	
};