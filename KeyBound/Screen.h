#pragma once

#include "Point.h"
#include <string>

class Screen {
public:
	static constexpr int MAX_X = 79;
	static constexpr int MAX_Y = 24;
private:

	char screen[MAX_Y + 1][MAX_X + 2];

	char backup[MAX_Y + 1][MAX_X + 2];

	bool hasBackup = false;

	char charAt(const Point& p) const {
		return screen[p.getY()][p.getX()];
	}
	

public:

	void saveBackup();
	void restoreBackup();

	bool loadFromFile(const std::string& filename);

	void setCell(int y, int x, char c) {
		screen[y][x] = c;
	}
	bool isWall(const Point& p) const {
		return charAt(p) == 'W';
	}
	bool isRiddle(const Point& p) const {
		return charAt(p) == '?';
	}
	bool isWonChar(const Point& p) const {
		return charAt(p) == '%';
	}
	bool isSpring(const Point& p) const {
		return charAt(p) == 'S';
	}
	void draw() const;

	char getCharAt(int y, int x) const {
		return screen[y][x];
	}
	
};